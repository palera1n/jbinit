SHELL := /usr/bin/env bash

all: ramdisk.dmg #binpack.dmg

jbinit: src/jbinit.c
	xcrun -sdk iphoneos clang -Os -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z --target=arm64-apple-ios7.0 -std=gnu17 -flto -ffreestanding -U__nonnull -nostdlibinc -fno-stack-protector src/jbinit.c src/printf.c -o jbinit
	mv jbinit com.apple.dyld
	ldid -S com.apple.dyld
	mv com.apple.dyld jbinit

jbloader: src/jbloader.c src/offsetfinder.c ent.xml
	xcrun -sdk iphoneos clang -miphoneos-version-min=7.0 -arch arm64 -Os src/jbloader.c src/offsetfinder.c -Isrc -o jbloader -pthread -flto=thin -Wl,-dead_strip -Wall -Wextra -funsigned-char -Wno-unused-parameter -framework IOKit -framework CoreFoundation -DLOADER_DMG_PATH=\"/private/var/palera1n.dmg\" -DLOADER_CHECKSUM=\"$(shell shasum -a 512 loader.dmg | cut -d' ' -f1)\" -DLOADER_SIZE=$(shell stat -f%z loader.dmg)L
	ldid -Sent.xml jbloader

jb.dylib: src/jb.c
	xcrun -sdk iphoneos clang -miphoneos-version-min=7.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -flto=thin -shared src/jb.c -o jb.dylib
	ldid -S jb.dylib

binpack.dmg: binpack
	rm -f ./binpack.dmg
	sudo mkdir -p binpack/Applications
	hdiutil create -size 8m -layout NONE -format UDZO -imagekey zlib-level=9 -srcfolder ./binpack -fs HFS+ ./binpack.dmg

ramdisk.dmg: jbinit jbloader jb.dylib
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
	mkdir -p ramdisk/usr/lib
	cp jbinit ramdisk/usr/lib/dyld
	cp jb.dylib jbloader ramdisk/jbin
	sudo gchown -R 0:0 ramdisk
	hdiutil create -size 512K -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ ./ramdisk.dmg

clean:
	rm -f jbinit launchd jb.dylib ramdisk.dmg binpack.dmg jbloader
	sudo rm -rf ramdisk
	rm -f ramdisk.img4

.PHONY: all clean
