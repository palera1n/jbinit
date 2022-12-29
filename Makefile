
# .PHONY: *

all: ramdisk.dmg

jbinit:
	xcrun -sdk iphoneos clang -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld -nostdlib -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z --target=arm64-apple-ios12.0 -std=gnu17 -flto -ffreestanding -U__nonnull -nostdlibinc -fno-stack-protector jbinit.c printf.c -o jbinit
	mv jbinit com.apple.dyld
	ldid -S com.apple.dyld
	mv com.apple.dyld jbinit

launchd:
	xcrun -sdk iphoneos clang -arch arm64 launchd.m -o launchd -fmodules -fobjc-arc
	ldid -Sent.xml launchd

jb.dylib:
	xcrun -sdk iphoneos clang -arch arm64 -shared jb.c -o jb.dylib
	ldid -S jb.dylib

ramdisk.dmg: jbinit launchd jb.dylib
	mkdir -p ramdisk
	mkdir -p ramdisk/dev
	mkdir -p ramdisk/sbin
	cp launchd ramdisk/sbin/launchd
	mkdir -p ramdisk/usr/lib
	cp jbinit ramdisk/usr/lib/dyld
	cp jb.dylib ramdisk/jb.dylib
	mkdir -p ramdisk/palera1n
	cp tar ramdisk/palera1n/tar
	cp wget ramdisk/palera1n/wget
	hdiutil create -size 8m -layout NONE -format UDRW -srcfolder ./ramdisk -fs HFS+ ./ramdisk.dmg

clean:
	rm -f jbinit
	rm -f launchd
	rm -f jb.dylib
	rm -f ramdisk.dmg
	rm -rf ramdisk
	rm -f ramdisk.img4
