all: jb.dylib

jb.dylib:
	$(CC) $(CFLAGS) $(LDFLAGS) -miphoneos-version-min=15.0 -arch arm64 -Os -Wall -Wextra -Wno-unused-parameter -shared jb.c -o jb.dylib
	ldid -S jb.dylib

.PHONY: jb.dylib all
