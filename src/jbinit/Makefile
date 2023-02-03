OBJECTS  = jbinit.o memcpy.o memset.o strcat.o strlen.o strstr.o
OBJECTS += syscall.o utils.o printf.o info.o

CFLAGS += -Os -nostdlib -fno-stack-protector -I.
CFLAGS += --target=arm64-apple-ios7.0 -std=gnu17 -ffreestanding -U__nonnull
LDFLAGS += -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld
LDFLAGS += -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z

all: jbinit

%.o: %.c
	$(CC) -c -nostdlibinc $(CFLAGS) $<

jbinit: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o jbinit
	$(STRIP) jbinit
	ldid -S -Icom.apple.dyld jbinit

