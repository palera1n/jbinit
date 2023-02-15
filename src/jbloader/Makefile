OBJECTS  = create_fakefs_sh.o enable_ssh.o fakefs.o lock_vars.o
OBJECTS += get_boot_manifest_hash.o info.o bakera1nd.o launchd.o
OBJECTS += jbloader.o mount_dmg.o obliterator.o p1ctl.o main.o
OBJECTS += prep_launch.o remount.o runcmd.o safemode_alert.o
OBJECTS += sbreload.o spin.o uicache.o sysstatuscheck.o print_info.o
OBJECTS += get_set_flags.o

CFLAGS  += -miphoneos-version-min=7.0 -arch arm64 -Os -I.
LDFLAGS += $(SRC)/APFS.tbd -framework IOKit -framework CoreFoundation -framework Foundation

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
endif

all: jbloader

%.o: %.c
	$(CC) -c $(CFLAGS) $<

uicache.o: uicache.m
	$(CC) -c $(CFLAGS) uicache.m

create_fakefs_sh.c: create_fakefs.sh
	xxd -iC create_fakefs.sh > create_fakefs_sh.c

jbloader: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o jbloader
	$(STRIP) jbloader
ifeq ($(ASAN),1)
	$(I_N_T) -add_rpath /cores jbloader
endif
	ldid -S$(SRC)/ent.xml -Icom.apple.jbloader jbloader

.PHONY: all
