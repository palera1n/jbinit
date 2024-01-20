SUBDIRS = fakedyld rootlesshooks payload_dylib payload systemhook ellekit rootfulhooks mount_cores
CFLAGS += -arch arm64 -I$(ROOT)/include -Wno-deprecated-declarations -Os -g
STRIP = STRIP
DSYMUTIL = dsymutil
LDID = ldid

ifneq ($(ASAN),1)
RAMDISK_SIZE = 2m
else
RAMDISK_SIZE = 8m
INCLUDE_ASAN = 1
endif

ifeq ($(INCLUDE_ASAN),1)
RAMDISK_SIZE = 8m
CFLAGS += -DASAN=1
endif

ifeq ($(DEV_BUILD),1)
CFLAGS += -DDEV_BUILD=1
endif

UID != id -u
ifeq ($(UID),0)
SUDO = sudo
else
PASSWORD != osascript -e 'Tell application "System Events" to display dialog "Password:" default answer "" with hidden answer' -e 'text returned of result' 2>/dev/null
SUDO = echo "$(PASSWORD)" | sudo -S
endif

.PHONY: all clean $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS)) libjailbreak

export CC STRIP DSYMUTIL CFLAGS LDID

all: ramdisk.dmg binpack.dmg

ramdisk.dmg: $(patsubst %, %-all, $(SUBDIRS))
	rm -rf ramdisk.dmg ramdisk
	mkdir -p ramdisk/{sbin,usr/lib,dev,binpack,fs/{real,fake},cores}
	install -m755 fakedyld/dyld ramdisk/
	install -m755 mount_cores/mount_cores.[0-9] ramdisk/
	install -m755 payload_dylib/payload.dylib ramdisk/
	install -m755 payload/payload ramdisk/
ifeq ($(INCLUDE_ASAN),1)
	install -m755 $(shell xcode-select -p)/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/*/lib/darwin/libclang_rt.asan_ios_dynamic.dylib ramdisk/
endif
	ln -sf /sbin/launchd ramdisk/cores/ploosh
	ln -sf ../payload ramdisk/sbin/launchd
	ln -sf ../../dyld ramdisk/usr/lib/dyld
	ln -sf payload ramdisk/ploosh
	hdiutil create -fs HFS+ -layout NONE -format UDRW -size $(RAMDISK_SIZE) -srcdir ramdisk -volname plooshra1nrd ramdisk.dmg

loader.dmg: palera1nLoader.ipa
	rm -rf loader.dmg palera1nLoader
	mkdir palera1nLoader
	cd palera1nLoader && unzip ../palera1nLoader.ipa
	hdiutil create -size 2m -layout NONE -format ULFO -volname palera1nLoader -srcfolder ./palera1nLoader/Payload -fs HFS+ ./loader.dmg
	rm -rf palera1nLoader

binpack.dmg: binpack.tar loader.dmg $(patsubst %, %-all, $(SUBDIRS))
	$(SUDO) rm -rf binpack.dmg binpack
	mkdir binpack
	$(SUDO) touch binpack/.installed_overlay
	$(SUDO) tar -C binpack --preserve-permissions -xf binpack.tar
	$(SUDO) rm -rf binpack/usr/share cores
	$(SUDO) ln -sf /cores/payload binpack/usr/sbin/p1ctl
	$(SUDO) ln -sf /cores/payload binpack/usr/sbin/palera1nd
	$(SUDO) mkdir -p binpack/Applications
	$(SUDO) mkdir -p binpack/usr/lib binpack/usr/share
	$(SUDO) mkdir -p binpack/Library/Frameworks/CydiaSubstrate.framework
	$(SUDO) mkdir -p binpack/Library/LaunchDaemons
	$(SUDO) cp -a $(ROOT)/shared/*.plist binpack/Library/LaunchDaemons
	$(SUDO) cp -a $(ROOT)/shared/boot.jp2 binpack/usr/share
	$(SUDO) cp -a $(ROOT)/shared/copyright binpack/usr/share
	$(SUDO) cp -a $(ROOT)/LICENSE binpack/usr/share/copyright/Licnese-plinit.txt
	$(SUDO) chown 0:0 binpack/Library/LaunchDaemons/*.plist
	$(SUDO) chmod 644 binpack/Library/LaunchDaemons/*.plist
	$(SUDO) cp rootlesshooks/rootlesshooks.dylib binpack/usr/lib
	$(SUDO) cp rootfulhooks/rootfulhooks.dylib binpack/usr/lib
	$(SUDO) cp systemhook/systemhook.dylib binpack/usr/lib
	$(SUDO) cp loader.dmg binpack
	$(SUDO) cp ellekit/ellekit_build/Build/Products/Release-iphoneos/libellekit.dylib binpack/usr/lib
	$(SUDO) ln -s ../../../usr/lib/libellekit.dylib binpack/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate
	$(SUDO) hdiutil create -size 10m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -volname plooshra1nfs -fs HFS+ ./binpack.dmg
	$(SUDO) chown $(UID) binpack.dmg
	$(SUDO) rm -rf binpack

libjailbreak:
	$(MAKE) -C libjailbreak all

$(patsubst %, %-all, $(SUBDIRS)): libjailbreak
	$(MAKE) -C $$(echo $@ | cut -d- -f1) all

clean: $(patsubst %, %-clean, $(SUBDIRS))
	rm -rf binpack.dmg ramdisk.dmg palera1nLoader binpack ramdisk

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $$(echo $@ | cut -d- -f1) clean
	$(MAKE) -C libjailbreak clean
	rm -rf ramdisk ramdisk.dmg

