#!/cores/binpack/bin/sh

set -e;

export PATH="/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/sbin:/cores/binpack/bin:/usr/bin:/usr/sbin:/bin:/sbin";
if [ "$1" = "" ]; then
    >&2 echo "Usage: $0 <fake rootdev> [partial: 0|1]";
    exit 1;
fi

fake_rootdev="$1";
partial="$2"

echo "** palera1n fakefs setup **";

echo "** Executing boot commands **";
/sbin/fsck -qL
/sbin/mount -P 1
/usr/libexec/init_data_protection
/sbin/mount -P 2
/usr/libexec/keybagd --init

echo "** testing for real root device **";

if [ -b "/dev/disk0s1s1" ]; then
    real_rootdev="/dev/disk0s1s1";
elif [ -b "/dev/disk1s1" ]; then
    real_rootdev="/dev/disk1s1";
else
    >&2 echo "Error: real root device not found";
    exit 1;
fi

echo "** Got real rootdev $real_rootdev **";

echo "** creating fakefs $fake_rootdev **";
if [ -b "$fake_rootdev" ]; then
    # should never exists, as if allow existing is set to true
    # jbloader would have deleted it still
    >&2 echo "Error: fakefs already exists";
    exit 1;
else
    /sbin/newfs_apfs -A -D -o role=r -v Xystem "/dev/disk0s1";
fi

sleep 2;

if ! [ -b "$fake_rootdev" ]; then
    >&2 echo "Error: fake root device did not exist even after supposed creation";
    exit 1;
fi

echo "** mounting realfs $real_rootdev **";
#/sbin/mount_apfs -o ro "$real_rootdev" /cores/fs/real
/cores/binpack/usr/bin/snaputil -s $(/cores/binpack/usr/bin/snaputil -o) / /cores/fs/real

echo "** mounting fakefs $fake_rootdev **";
/sbin/mount_apfs -o rw "$fake_rootdev" /cores/fs/fake

echo "** copying files to fakefs (may take up to 10 minutes) **";
if [ "$partial" != "1" ]; then
    cp -a /cores/fs/real/* /cores/fs/fake/
    cp -a /cores/fs/real/.mb /cores/fs/real/.file /cores/fs/real/.ba /cores/fs/fake/
    cp -a /cores/fs/real/.fseventsd /cores/fs/fake/ 2> /dev/null || true
else
    cd /cores/fs/real
    cp -a .ba .file .mb Applications Developer Library bin cores dev private sbin usr etc tmp var /cores/fs/fake
    rm -rf /cores/fs/fake/usr/standalone/update
    mkdir -p /cores/fs/fake/usr/standalone/update
    mkdir -p /cores/fs/fake/System/Library
    if [ "$real_rootdev" = "/dev/disk1s1" ]; then
        cp -a /System/Applications /System/Cryptexes /cores/fs/fake/System
    fi
    cp -a /System/Developer /System/DriverKit /cores/fs/fake/System
    for filepath in /cores/fs/real/System/Library/*; do
        if [ -d "$filepath" ]; then
            if [ "$filepath" = "/cores/fs/real/System/Library/Frameworks" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/AccessibilityBundles" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Assistant" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Audio" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Caches" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Fonts" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Health" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/LinguisticData" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/OnBoardingBundles" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/Photos" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/PreferenceBundles" ] ||
              [ "$filepath" = "/cores/fs/real/System/Library/PreinstalledAssetsV2" ]; then
                newpath="$(echo "$filepath" | sed 's|/cores/fs/real|/cores/fs/fake|')"
                echo "mkdir $newpath"
                mkdir "$newpath"
            elif [ "$filepath" = "/cores/fs/real/System/Library/PrivateFrameworks" ] ||
                [ "$filepath" = "/cores/fs/real/System/Library/Caches" ]; then
                if [ "$real_rootdev" = "/dev/disk1s1" ]; then
                    echo "copy $filepath -> /cores/fs/fake/System/Library"
                    cp -a "$filepath" /cores/fs/fake/System/Library
                else
                    newpath="$(echo "$filepath" | sed 's|/cores/fs/real|/cores/fs/fake|')"
                    echo "mkdir $newpath"
                    mkdir "$newpath"
                fi
            else
                echo "copy $filepath -> /cores/fs/fake/System/Library"
                cp -a "$filepath" /cores/fs/fake/System/Library
            fi
        fi
    done
fi
if [ "$real_rootdev" = "/dev/disk1s1" ]; then
    rm -rf /cores/fs/fake/System/Library/Caches/com.apple.dyld
    ln -s /System/Cryptexes/OS/System/Library/Caches/com.apple.dyld /cores/fs/fake/System/Library/Caches/
fi
echo "** syncing filesystems **";
sync

sleep 2;

echo "** unmounting filesystems **";
/sbin/umount -f /cores/fs/real
/sbin/umount -f /cores/fs/fake
/sbin/umount -a || true

sync

echo "** Done setting up fakefs **";
rm -f /cores/setup_fakefs.sh
exit 0;
