all: jb.dylib injector.dylib libellekit.dylib rootlesshooks.dylib

jb.dylib: jb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared jb.c -o jb.dylib
	ldid -S jb.dylib
	
injector.dylib: injector.c
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared injector.c -framework Foundation -o injector.dylib
	ldid -S injector.dylib

libellekit.dylib: ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib
	cp ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib .
	$(I_N_T) -id '/cores/binpack/usr/lib/libelegantlowlevelelements.dylib' libellekit.dylib
	ldid -S libellekit.dylib

ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib:
	cd ellekit; \
		CC="" CXX="" LD="" xcodebuild build -arch arm64 -sdk iphoneos IPHONEOS_DEPLOYMENT_TARGET=15.0 CODE_SIGNING_ALLOWED=NO -scheme ellekit -configuration Release -derivedDataPath build

rootlesshooks.dylib: rootlesshooks/.theos/obj/rootlesshooks.dylib
	cp rootlesshooks/.theos/obj/rootlesshooks.dylib rootlesshooks.dylib
	$(I_N_T) -change '@rpath/CydiaSubstrate.framework/CydiaSubstrate' '@rpath/libelegantlowlevelelements.dylib' rootlesshooks.dylib
	$(I_N_T) -add_rpath /cores/binpack/usr/lib rootlesshooks.dylib
	$(I_N_T) -id '/cores/binpack/usr/lib/rootlesshooks.dylib' rootlesshooks.dylib
	ldid -S rootlesshooks.dylib

rootlesshooks/.theos/obj/rootlesshooks.dylib:
	$(MAKE) -C rootlesshooks all

.PHONY: libellekit.dylib ellekit/build/Build/Products/Release-iphoneos/libellekit.dylib rootlesshooks.dylib rootlesshooks/.theos/obj/rootlesshooks.dylib all
