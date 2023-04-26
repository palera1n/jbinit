all: jb.dylib injector.dylib

jb.dylib:
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared jb.c -o jb.dylib
	ldid -S jb.dylib
	
injector.dylib:
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared injector.c -framework Foundation -o injector.dylib
	ldid -S injector.dylib

.PHONY: jb.dylib injector.dylib all
