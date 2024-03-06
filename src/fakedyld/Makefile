SUBDIRS = asm lib main patch_dyld
CFLAGS += -nostdlib -nostdlibinc -fno-stack-protector -fomit-frame-pointer
CFLAGS += --target=arm64-apple-tvos12.0 -std=gnu17 -ffreestanding -U__nonnull
LDFLAGS += -e__dyld_start -Wl,-dylinker -Wl,-dylinker_install_name,/usr/lib/dyld
LDFLAGS += -static -Wl,-fatal_warnings -Wl,-dead_strip -Wl,-Z -Wno-unused-command-line-argument

export CFLAGS LDFLAGS

all: $(patsubst %, %-all, $(SUBDIRS)) dyld

clean: $(patsubst %, %-clean, $(SUBDIRS))
	rm -rf dyld dyld.dSYM

dyld: $(patsubst %, %-all, $(SUBDIRS))
	$(CC) $(CFLAGS) $(LDFLAGS) $$(find . -name '*.o' | grep 'obj/.*o') -o dyld
	$(DSYMUTIL) $@
ifeq ($(DEV_BUILD),1)
	$(STRIP) $@
endif
	$(LDID) -Icom.apple.dyld -S $@

$(patsubst %, %-all, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1)

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1) clean

.PHONY: all $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS))
