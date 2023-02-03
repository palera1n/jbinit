OBJECTS  = create_fakefs_sh.o enable_ssh.o fakefs.o
OBJECTS += get_boot_manifest_hash.o info.o jbloader.o launchd.o
OBJECTS += main.o mount_dmg.o obliterator.o offsetfinder.o
OBJECTS += prep_launch.o remount.o runcmd.o safemode_alert.o
OBJECTS += sbreload.o spin.o uicache.o mount.o

CFLAGS  += -miphoneos-version-min=7.0 -arch arm64 -Os -I.
LDFLAGS += $(SRC)/APFS.tbd -framework IOKit -framework CoreFoundation

all: jbloader

%.o: %.c
	$(CC) -c $(CFLAGS) $<

create_fakefs_sh.c: create_fakefs.sh
	xxd -iC create_fakefs.sh > create_fakefs_sh.c

jbloader: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o jbloader
	ldid -S$(SRC)/ent.xml -Icom.apple.jbloader jbloader

.PHONY: all
