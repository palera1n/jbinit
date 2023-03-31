OBJECTS  = jbinit.o prepare_rootfs.o select_root.o syscall.o
OBJECTS += utils.o info.o mount_devfs.o rw_file.o rootwait.o
OBJECTS += remount_rdisk.o mountroot.o fakefs_check.o init_log.o
OBJECTS += cores_init.o log.o

SUBDIRS = patch_dyld lib

CFLAGS += -Os -nostdlib -fno-stack-protector -I.
CFLAGS += --target=arm64-apple-ios15.0 -std=gnu17 -ffreestanding -U__nonnull
LDFLAGS += -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld
LDFLAGS += -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z

all: jbinit

%.o: %.c
	$(CC) -c -nostdlibinc $(CFLAGS) $<

$(SUBDIRS):
	$(MAKE) -C $@

jbinit: $(OBJECTS) $(SUBDIRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(patsubst %.c, %.o, $(wildcard lib/*.c patch_dyld/*.c)) -o jbinit
	$(STRIP) jbinit
	ldid -S -Icom.apple.dyld jbinit

.PHONY: all $(SUBDIRS)
