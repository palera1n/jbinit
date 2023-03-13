OBJECTS  = jbinit.o plooshfinder.o prepare_rootfs.o select_root.o
OBJECTS += syscall.o utils.o printf.o info.o mount_devfs.o rw_file.o
OBJECTS += remount_rdisk.o mountroot.o fakefs_check.o init_log.o
OBJECTS += dyld_platform_old.o dyld_platform_ios15.o dyld_platform_ios16.o
OBJECTS += cores_init.o rootwait.o patch_dyld.o

CFLAGS += -Os -nostdlib -fno-stack-protector -I.
CFLAGS += --target=arm64-apple-ios7.0 -std=gnu17 -ffreestanding -U__nonnull
LDFLAGS += -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld
LDFLAGS += -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z

all: jbinit

%.o: %.c
	$(CC) -c -nostdlibinc $(CFLAGS) $<

lib:
	$(MAKE) -C lib

jbinit: $(OBJECTS) lib
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(patsubst %.c, %.o, $(wildcard lib/*.c)) -o jbinit
	$(STRIP) jbinit
	ldid -S -Icom.apple.dyld jbinit

.PHONY: all lib
