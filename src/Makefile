CFLAGS += -arch arm64 -I$(ROOT)/include -Wno-deprecated-declarations -g
SHELL = /usr/bin/env bash

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
CFLAGS += -O0 -DDEV_BUILD=1 -DHAVE_DEBUG_SHELL=1 -DHAVE_DEBUG_JBD_MSG=1
else
CFLAGS += -Oz -flto=full
endif

CREATE_UDRW_IMAGE = \
	set -e; \
	if [ "$$MAC" = "1" ]; then \
		dd if=/dev/zero of=$(1) bs=1M count=$(2); \
		IMAGE_DEV="$$(hdiutil attach -nomount -imagekey diskimage-class=CRawDiskImage $(1))"; \
	elif [ "$$UNAME" = "Darwin" ]; then \
		IMAGE_DEV="$$(hdik -drivekey diskimage-class=CRawDiskImage -nomount "ram://1024")"; \
	else \
		dd if=/dev/zero of=$(1) bs=1M count=$(2); \
		IMAGE_DEV="$(1)"; \
	fi; \
	$(NEWFS_HFS) -s -G 0 -U 0 -v $(3) $$IMAGE_DEV; \
        if [ "$$MAC" = "1" ]; then \
		hdiutil detach $$IMAGE_DEV; \
	elif [ "$$UNAME" = "Darwin" ]; then \
		dd if=$$IMAGE_DEV of=$(1); \
		$(HFSPLUS) $(1) grow $$(expr $(2) '*' 1048576); \
		hdik -e $$IMAGE_DEV; \
	fi; \
	$(FAKEROOT) $(HFSPLUS) $(1) addall $(4);

.PHONY: all clean $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS)) libjailbreak

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
ifeq ($(INCLUDE_ASAN),1)
	$(call CREATE_UDRW_IMAGE,ramdisk.dmg,8,plooshra1nrd,ramdisk)
else
	$(call CREATE_UDRW_IMAGE,ramdisk.dmg,2,plooshra1nrd,ramdisk)
endif
loader.dmg: palera1nLoader.ipa
	rm -rf loader.dmg palera1nLoader
	mkdir palera1nLoader
	cd palera1nLoader && unzip ../palera1nLoader.ipa
	$(call CREATE_UDRW_IMAGE,loader.img,2,palera1nLoader,palera1nLoader/Payload)
ifeq ($(MAC),1)
	hdiutil convert -format ULFO loader.img -o loader.dmg
else
	$(DMG) dmg loader.img loader.dmg -zlib-level 9
endif
	rm -rf palera1nLoader

binpack.dmg: binpack.tar loader.dmg $(patsubst %, %-all, $(SUBDIRS))
	rm -rf binpack.dmg binpack.img binpack
	mkdir binpack
	touch binpack/.installed_overlay
	tar -C binpack --preserve-permissions -xf binpack.tar
	rm -rf binpack/usr/share cores
	ln -sf /cores/payload binpack/usr/sbin/p1ctl
	ln -sf /cores/payload binpack/usr/sbin/palera1nd
	mkdir -p binpack/Applications
	mkdir -p binpack/usr/lib binpack/usr/share
	mkdir -p binpack/Library/Frameworks/CydiaSubstrate.framework
	mkdir -p binpack/Library/LaunchDaemons
	cp -a $(ROOT)/shared/*.plist binpack/Library/LaunchDaemons
	cp -a $(ROOT)/shared/boot.jp2 binpack/usr/share
	cp -a $(ROOT)/shared/copyright binpack/usr/share
	cp -a $(ROOT)/LICENSE binpack/usr/share/copyright/Licnese-plinit.txt
	chmod 644 binpack/Library/LaunchDaemons/*.plist
	cp rootlesshooks/rootlesshooks.dylib binpack/usr/lib
	cp rootfulhooks/rootfulhooks.dylib binpack/usr/lib
	cp systemhook/systemhook.dylib binpack/usr/lib
	cp loader.dmg binpack
	cp ellekit/ellekit_build/Build/Products/Release-iphoneos/libellekit.dylib binpack/usr/lib
	ln -s ../../../usr/lib/libellekit.dylib binpack/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate
	$(call CREATE_UDRW_IMAGE,binpack.img,10,plooshra1nfs,binpack)
ifeq ($(MAC),1)
	hdiutil convert -format UDZO -imagekey zlib-level=9 binpack.img -o binpack.dmg
else
	$(DMG) dmg binpack.img binpack.dmg -zlib-level 9
endif
	rm -rf binpack

libjailbreak:
	$(MAKE) -C libjailbreak all

$(patsubst %, %-all, $(SUBDIRS)): libjailbreak
	$(MAKE) -C $$(echo $@ | cut -d- -f1) all

clean: $(patsubst %, %-clean, $(SUBDIRS))
	rm -rf binpack.img ramdisk.img loader.img binpack.dmg ramdisk.dmg loader.dmg palera1nLoader binpack ramdisk

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $$(echo $@ | cut -d- -f1) clean
	$(MAKE) -C libjailbreak clean
	rm -rf ramdisk ramdisk.dmg

distclean:
	$(MAKE) -C ellekit distclean
