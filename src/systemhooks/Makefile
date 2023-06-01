all: payload.dylib rootlesshooks.dylib libellekit.dylib

payload.dylib: payload.c
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared payload.c -o payload.dylib
	ldid -S payload.dylib

libellekit.dylib: ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib
	cp ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib .
	ldid -S libellekit.dylib

ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib:
	cd ellekit; \
		CC="" CXX="" LD="" xcodebuild build -arch arm64 -sdk iphoneos -scheme ellekit -configuration Release -derivedDataPath build \
			IPHONEOS_DEPLOYMENT_TARGET=15.0 \
			CODE_SIGNING_ALLOWED=NO \
			LD_DYLIB_INSTALL_NAME="@rpath/CydiaSubstrate.framework/CydiaSubstrate"

cfprefsd.o: cfprefsd.c
	$(CC) $(CFLAGS) $(LDFLAGS) -flto=thin cfprefsd.c -c -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter

rootlesshooks.dylib: cfprefsd.o libellekit.dylib
	xcrun -sdk iphoneos swiftc -Osize cfprefsd.o libellekit.dylib \
		-lto=llvm-thin \
		--target=arm64-apple-ios15.0 \
		-sdk $(shell xcrun -sdk iphoneos --show-sdk-path) \
		-Xlinker -rpath -Xlinker /var/jb/Library/Frameworks \
		-Xlinker -rpath -Xlinker /var/jb/usr/lib \
		-Xlinker -rpath -Xlinker /cores/binpack/Library/Frameworks \
		-Xlinker -rpath -Xlinker /cores/binpack/usr/lib \
		-Xlinker -install_name -Xlinker /cores/binpack/usr/lib/rootlesshooks.dylib \
		-Xlinker -dead_strip \
		-emit-library \
		-framework Foundation \
		-framework CoreFoundation \
		-lswiftCore \
		-o rootlesshooks.dylib
	ldid -S rootlesshooks.dylib 

.PHONY: libellekit.dylib ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib
