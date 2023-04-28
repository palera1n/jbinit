all: jb.dylib injector.dylib rootlesshooks.dylib

jb.dylib: jb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared jb.c -o jb.dylib
	ldid -S jb.dylib
	
injector.dylib: injector.c
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared injector.c -framework Foundation -o injector.dylib
	ldid -S injector.dylib

libellekit.a: ellekit/build/Build/Products/Release-iphoneos/libellekit.a
	cp ellekit/build/Build/Products/Release-iphoneos/libellekit.a .

ellekit/build/Build/Products/Release-iphoneos/libellekit.a:
	cd ellekit; \
		CC="" CXX="" LD="" xcodebuild build -arch arm64 -sdk iphoneos -scheme ellekit -configuration Release -derivedDataPath build \
			IPHONEOS_DEPLOYMENT_TARGET=15.0 \
			CODE_SIGNING_ALLOWED=NO \
			MACH_O_TYPE="staticlib" \
			EXECUTABLE_NAME="libellekit.a" \
			EXECUTABLE_PATH="libellekit.a" \
			EXECUTABLE_EXTENSION="a" \
			EXECUTABLE_PREFIX="lib" \
			EXECUTABLE_SUFFIX=".a" \
			LLVM_LTO=YES

cfprefsd.o: cfprefsd.c
	$(CC) $(CFLAGS) $(LDFLAGS) -flto=thin cfprefsd.c -c -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter

rootlesshooks.dylib: cfprefsd.o libellekit.a
	swiftc -Osize cfprefsd.o libellekit.a -lto=llvm-thin --target=arm64-apple-ios15.0 -sdk $(shell xcrun -sdk iphoneos --show-sdk-path) -Osize -Xlinker -dead_strip -emit-library -framework Foundation -framework CoreFoundation -o rootlesshooks.dylib
	$(I_N_T) -id '/cores/binpack/usr/lib/rootlesshooks.dylib' rootlesshooks.dylib
	ldid -S rootlesshooks.dylib

.PHONY: libellekit.a ellekit/build/Build/Products/Release-iphoneos/libellekit.a
