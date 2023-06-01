SHELL := /usr/bin/env bash
SRC = $(shell pwd)/src
CC = xcrun -sdk iphoneos clang
STRIP = xcrun -sdk iphoneos strip
I_N_T = install_name_tool
CFLAGS += -I$(SRC) -I$(SRC)/include -flto=full
ifeq ($(ASAN),1)
CFLAGS += -DASAN
endif
ifeq ($(DEV_BUILD),1)
CFLAGS += -DDEV_BUILD
DEV_TARGETS += xpchook.dylib
endif

ifeq ($(ASAN),1)
RAMDISK_SIZE = 8M
else ifeq ($(DEV_BUILD),1)
RAMDISK_SIZE = 1M
else
RAMDISK_SIZE = 512K
endif

export SRC CC CFLAGS LDFLAGS STRIP I_N_T

all: ramdisk.dmg

binpack.dmg: binpack.tar loader.dmg hook_all
	sudo rm -rf ./binpack.dmg binpack
	sudo mkdir binpack
	sudo tar -C binpack --preserve-permissions -xf binpack.tar
	sudo rm -rf binpack/usr/share cores
	sudo ln -sf /cores/jbloader binpack/usr/sbin/p1ctl
	sudo mkdir -p binpack/Applications
	sudo mkdir -p binpack/usr/lib
	sudo mkdir -p binpack/Library/Frameworks/CydiaSubstrate.framework
	sudo mkdir -p binpack/Library/LaunchDaemons/DropBear/
	sudo cp -a LaunchDaemons/DropBear/*.plist binpack/Library/LaunchDaemons/DropBear/
	sudo cp src/systemhooks/rootlesshooks.dylib binpack/usr/lib
	sudo cp loader.dmg binpack
	sudo cp src/systemhooks/libellekit.dylib binpack/usr/lib
	sudo ln -s ../../../usr/lib/libellekit.dylib binpack/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate
	sudo chown -R 0:0 binpack
	hdiutil create -size 8m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -volname palera1nfs -fs HFS+ ./binpack.dmg
	sudo rm -rf binpack

ramdisk.dmg: jbinit jbloader payload.dylib $(DEV_TARGETS)
	$(MAKE) -C $(SRC)
	rm -f ramdisk.dmg
	sudo rm -rf ramdisk
	mkdir -p ramdisk
	mkdir -p ramdisk/{usr/lib,sbin,jbin,dev,mnt}
	ln -s /jbin/jbloader ramdisk/sbin/launchd
	ln -s /sbin/launchd ramdisk/jbin/launchd
	mkdir -p ramdisk/usr/lib
	cp $(SRC)/jbinit/jbinit ramdisk/usr/lib/dyld
	cp $(SRC)/systemhooks/payload.dylib $(SRC)/jbloader/jbloader ramdisk/jbin
ifeq ($(DEV_BUILD),1)
	cp $(SRC)/jbloader/launchctl/tools/xpchook.dylib ramdisk/jbin
endif
ifeq ($(ASAN),1)
	cp $(shell xcode-select -p)/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/*//lib/darwin/libclang_rt.{asan,ubsan}_ios_dynamic.dylib ramdisk/jbin
endif
	sudo chown -R 0:0 ramdisk
	hdiutil create -size $(RAMDISK_SIZE) -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ -volname palera1nrd ./ramdisk.dmg

loader.dmg: palera1nLoader.ipa
	rm -rf loader.dmg Payload
	unzip palera1nLoader.ipa
	hdiutil create -size 1m -layout NONE -format ULFO -uid 0 -gid 0 -volname palera1nLoader -srcfolder ./Payload -fs HFS+ ./loader.dmg
	rm -rf Payload

$(SRC)/dyld_platform_test/dyld_platform_test:
	$(MAKE) -C $(SRC)/dyld_platform_test

dyld_platform_test: $(SRC)/dyld_platform_test/dyld_platform_test

xpchook.dylib:
	$(MAKE) -C src/jbloader xpchook.dylib

clean:
	rm -f payload.dylib binpack.dmg src/launchctl/tools/xpchook.dylib src/systemhooks/libellekit.dylib ramdisk.dmg \
		src/jbinit/jbinit src/jbloader/jbloader src/systemhooks/payload.dylib \
		src/jbloader/loader/create_fakefs_sh.c src/dyld_platform_test/dyld_platform_test loader.dmg \
		src/systemhooks/rootlesshooks.dylib
	sudo rm -rf ramdisk binpack cores
	rm -rf src/systemhooks/ellekit/build src/systemhooks/rootlesshooks/.theos
	find . -name '*.o' -delete
	rm -f ramdisk.img4

hook_all:
	$(MAKE) -C src/systemhooks all

.PHONY: all clean jbinit jbloader payload.dylib dyld_platform_test xpchook.dylib binpack.dmg hook_all
