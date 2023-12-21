SUBDIRS = fakedyld rootlesshooks payload_dylib payload systemhook ellekit rootfulhooks mount_cores
CFLAGS += -arch arm64 -I$(ROOT)/include -Wno-deprecated-declarations -Os -g
CC = xcrun -sdk iphoneos clang
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
	sudo rm -rf binpack.dmg binpack
	mkdir binpack
	-sudo touch binpack/.installed_overlay
	sudo tar -C binpack --preserve-permissions -xf binpack.tar
	sudo rm -rf binpack/usr/share cores
	sudo ln -sf /cores/payload binpack/usr/sbin/p1ctl
	sudo ln -sf /cores/payload binpack/usr/sbin/palera1nd
	sudo mkdir -p binpack/Applications
	sudo mkdir -p binpack/usr/lib binpack/usr/share
	sudo mkdir -p binpack/Library/Frameworks/CydiaSubstrate.framework
	sudo mkdir -p binpack/Library/LaunchDaemons
	sudo cp -a $(ROOT)/shared/*.plist binpack/Library/LaunchDaemons
	sudo cp -a $(ROOT)/shared/boot.jp2 binpack/usr/share
	sudo chown 0:0 binpack/Library/LaunchDaemons/*.plist
	sudo chmod 644 binpack/Library/LaunchDaemons/*.plist
	sudo cp rootlesshooks/rootlesshooks.dylib binpack/usr/lib
	sudo cp rootfulhooks/rootfulhooks.dylib binpack/usr/lib
	sudo cp systemhook/systemhook.dylib binpack/usr/lib
	sudo cp loader.dmg binpack
	sudo cp ellekit/ellekit_build/Build/Products/Release-iphoneos/libellekit.dylib binpack/usr/lib
	sudo ln -s ../../../usr/lib/libellekit.dylib binpack/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate
	hdiutil create -size 10m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -volname plooshra1nfs -fs HFS+ ./binpack.dmg
	sudo rm -rf binpack

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

