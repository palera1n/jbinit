all: ramdisk.dmg

jbinit: jbinit.c payload.c payload.h
	xcrun -sdk iphoneos clang -Os -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z --target=arm64-apple-ios12.0 -std=gnu17 -flto -ffreestanding -U__nonnull -nostdlibinc -fno-stack-protector jbinit.c printf.c payload.c -o jbinit
	mv jbinit com.apple.dyld
	ldid -S com.apple.dyld
	mv com.apple.dyld jbinit

launchd: launchd.c
	xcrun -sdk iphoneos clang -arch arm64 -Os launchd.c -o launchd
	ldid -Sent.xml launchd

jbloader: jbloader.m
	xcrun -sdk iphoneos clang -arch arm64 -Os jbloader.m -o jbloader -fmodules -fobjc-arc
	ldid -Sent.xml jbloader

jb.dylib: jb.c
	xcrun -sdk iphoneos clang -arch arm64 -Os -shared jb.c -o jb.dylib
	ldid -S jb.dylib

payload.c: jb.dylib binpack.dmg jbloader
	xxd -iC jb.dylib > payload.c
	xxd -iC binpack.dmg >> payload.c
	xxd -iC jbloader >> payload.c

binpack.dmg: binpack
	rm -f ./binpack.dmg
	hdiutil create -size 16m -layout NONE -format UDZO -srcfolder ./binpack -fs HFS+ ./binpack.dmg

ramdisk.dmg: jbinit launchd
	rm -f ramdisk.dmg
	mkdir -p ramdisk
	mkdir -p ramdisk/dev
	mkdir -p ramdisk/sbin
	cp launchd ramdisk/sbin/launchd
	mkdir -p ramdisk/usr/lib
	cp jbinit ramdisk/usr/lib/dyld
	hdiutil create -size 4m -layout NONE -format UDRW -srcfolder ./ramdisk -fs HFS+ ./ramdisk.dmg

clean:
	rm -f jbinit launchd jb.dylib ramdisk.dmg binpack.dmg jbloader
	rm -rf ramdisk
	rm -f ramdisk.img4

.PHONY: all clean
