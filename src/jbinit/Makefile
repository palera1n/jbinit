OBJECTS  = jbinit.o memcpy.o memset.o strcat.o strlen.o strstr.o
OBJECTS += syscall.o utils.o printf.o info.o mount_devfs.o rw_file.o
OBJECTS += remount_rdisk.o mountroot.o fakefs_check.o init_log.o
OBJECTS += cores_init.o rootwait.o select_root.o

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

