SHELL := /usr/bin/env bash

all: ramdisk.dmg

jbinit: jbinit.c
	xcrun -sdk iphoneos clang -Os -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z --target=arm64-apple-ios12.0 -std=gnu17 -flto -ffreestanding -U__nonnull -nostdlibinc -fno-stack-protector jbinit.c printf.c -o jbinit
	mv jbinit com.apple.dyld
	ldid -S com.apple.dyld
	mv com.apple.dyld jbinit

launchd: launchd.c ent.xml
	xcrun -sdk iphoneos clang -arch arm64 -Os launchd.c -o launchd
	ldid -Sent.xml launchd

jbloader: jbloader.m ent.xml
	xcrun -sdk iphoneos clang -arch arm64 -Os jbloader.m -o jbloader -fmodules -fobjc-arc
	ldid -Sent.xml jbloader

jb.dylib: jb.c
	xcrun -sdk iphoneos clang -arch arm64 -Os -shared jb.c -o jb.dylib
	ldid -S jb.dylib

binpack.dmg: binpack
	rm -f ./binpack.dmg
	hdiutil create -size 16m -layout NONE -format UDZO -srcfolder ./binpack -fs HFS+ ./binpack.dmg

ramdisk.dmg: jbinit launchd jbloader jb.dylib
	rm -f ramdisk.dmg
	sudo rm -rf ramdisk
	mkdir -p ramdisk
	mkdir -p ramdisk/{jbin,fs/{gen,orig}}
	mkdir -p ramdisk/{Applications,bin,cores,dev,Developer,Library,private,sbin,System,usr/lib}
	cp -a binpack ramdisk
	ln -s private/etc ramdisk/etc
	ln -s private/var ramdisk/var
	ln -s private/var/tmp ramdisk/tmp
	touch ramdisk/.file
	cp launchd ramdisk/sbin/launchd
	mkdir -p ramdisk/usr/lib
	cp jbinit ramdisk/usr/lib/dyld
	cp jb.dylib jbloader ramdisk/jbin
	sudo gchown -R 0:0 ramdisk
	hdiutil create -size 16m -layout NONE -format UDRW -uid 0 -gid 0 -srcfolder ./ramdisk -fs HFS+ ./ramdisk.dmg

clean:
	rm -f jbinit launchd jb.dylib ramdisk.dmg binpack.dmg jbloader
	sudo rm -rf ramdisk
	rm -f ramdisk.img4

.PHONY: all clean
