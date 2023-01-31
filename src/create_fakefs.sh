#!/cores/binpack/bin/sh

set -e;

export PATH="/cores/binpack/usr/bin:/cores/binpack/usr/sbin:/cores/binpack/sbin:/cores/binpack/bin:/usr/bin:/usr/sbin:/bin:/sbin";
if [ "$1" = "" ]; then
    >&2 echo "Usage: $0 <fake rootdev>";
    exit 1;
fi

fake_rootdev="$1";

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
/sbin/mount_apfs -o ro "$real_rootdev" /cores/fs/real

echo "** mounting fakefs $fake_rootdev **";
/sbin/mount_apfs -o rw "$fake_rootdev" /cores/fs/fake

echo "** copying files to fakefs (may take up to 10 minutes) **";
cp -a /cores/fs/real/. /cores/fs/fake/

echo "** syncing filesystems **";
sync

sleep 2;

echo "** unmounting filesystems **";
/sbin/umount -f /cores/fs/real
/sbin/umount -f /cores/fs/fake
/sbin/umount -a || true

echo "** Done setting up fakefs **";
rm -f /cores/setup_fakefs.sh
exit 0;
