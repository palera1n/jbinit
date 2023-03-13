SHELL := /usr/bin/env bash
SRC = $(shell pwd)/src
CC = xcrun -sdk iphoneos clang
STRIP = strip
I_N_T = install_name_tool
CFLAGS += -I$(SRC) -I$(SRC)/include -flto=full
ifeq ($(ASAN),1)
CFLAGS += -DASAN
endif
export SRC CC CFLAGS STRIP I_N_T

all: ramdisk.dmg

binpack.dmg: binpack loader.dmg
	rm -f ./binpack.dmg
	sudo rm -rf binpack/usr/share
	sudo ln -sf /cores/jbloader binpack/usr/sbin/p1ctl
	sudo mkdir -p binpack/Applications
	sudo cp loader.dmg binpack
	sudo chown -R 0:0 binpack
	hdiutil create -size 10m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -volname palera1nfs -fs HFS+ ./binpack.dmg

ramdisk.dmg: jbinit jbloader jb.dylib
	$(MAKE) -C $(SRC)
	rm -f ramdisk.dmg
	sudo rm -rf ramdisk
	mkdir -p ramdisk
	mkdir -p ramdisk/{usr/lib,sbin,jbin,dev}
	ln -s /jbin/jbloader ramdisk/sbin/launchd
	ln -s /sbin/launchd ramdisk/jbin/launchd
	mkdir -p ramdisk/usr/lib
	cp $(SRC)/jbinit/jbinit ramdisk/usr/lib/dyld
	cp $(SRC)/launchd_hook/jb.dylib $(SRC)/jbloader/jbloader ramdisk/jbin
ifeq ($(ASAN),1)
	cp $(shell xcode-select -p)/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/*//lib/darwin/libclang_rt.{asan,ubsan}_ios_dynamic.dylib ramdisk/jbin
endif
	sudo gchown -R 0:0 ramdisk
ifeq ($(ASAN),1)
	hdiutil create -size 8M -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ -volname palera1nrd ./ramdisk.dmg
else
	hdiutil create -size 512K -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ -volname palera1nrd ./ramdisk.dmg
endif

loader.dmg: palera1n.ipa
	rm -rf loader.dmg Payload
	unzip palera1n.ipa
	hdiutil create -size 10m -layout NONE -format ULFO -uid 0 -gid 0 -volname palera1nLoader -srcfolder ./Payload -fs HFS+ ./loader.dmg
	rm -rf Payload

clean:
	rm -f jb.dylib ramdisk.dmg binpack.dmg
	rm -f src/jbinit/jbinit src/jbloader/jbloader src/launchd_hook/jb.dylib
	rm -f src/jbloader/create_fakefs_sh.c
	sudo rm -rf ramdisk
	find . -name '*.o' -delete
	rm -f ramdisk.img4

.PHONY: all clean jbinit jbloader jb.dylib
