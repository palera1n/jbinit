SHELL := /usr/bin/env bash
SRC = $(shell pwd)/src
CC = xcrun -sdk iphoneos clang
CFLAGS += -I$(SRC) -flto=thin
export SRC CC CFLAGS

all: ramdisk.dmg

binpack.dmg: binpack loader.dmg
	rm -f ./binpack.dmg
	sudo rm -rf binpack/usr/share
	sudo mkdir -p binpack/Applications
	sudo cp loader.dmg binpack
	sudo chown -R 0:0 binpack
	hdiutil create -size 10m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -fs HFS+ ./binpack.dmg

ramdisk.dmg: jbinit jbloader jb.dylib
	$(MAKE) -C $(SRC)
	rm -f ramdisk.dmg
	sudo rm -rf ramdisk
	mkdir -p ramdisk
	mkdir -p ramdisk/{binpack,jbin,fs/{gen,orig}}
	mkdir -p ramdisk/{Applications,bin,cores,dev,Developer,Library,private,sbin,System,usr/lib}
	mkdir -p ramdisk/{.ba,.mb}
	ln -s private/etc ramdisk/etc
	ln -s private/var ramdisk/var
	ln -s private/var/tmp ramdisk/tmp
	touch ramdisk/.file
	chmod 000 ramdisk/.file
	chmod 700 ramdisk/{.ba,.mb}
	ln -s /jbin/jbloader ramdisk/sbin/launchd
	ln -s /sbin/launchd ramdisk/jbin/launchd
	mkdir -p ramdisk/usr/lib
	cp $(SRC)/jbinit/jbinit ramdisk/usr/lib/dyld
	cp $(SRC)/launchd_hook/jb.dylib $(SRC)/jbloader/jbloader ramdisk/jbin
	sudo gchown -R 0:0 ramdisk
	hdiutil create -size 512K -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ ./ramdisk.dmg

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
