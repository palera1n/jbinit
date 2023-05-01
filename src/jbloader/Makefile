OBJECTS  = create_fakefs_sh.o main.o launchctl/xpc_helper.o
OBJECTS += launchctl/bootstrap.o launchctl/load.o

SUBDIRS = loader p1ctl utils palera1nHelper

CFLAGS  += -miphoneos-version-min=15.0 -arch arm64 -Os -I.
LDFLAGS += $(SRC)/APFS.tbd -framework IOKit -framework CoreFoundation -framework Foundation -lobjc

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
endif

all: jbloader

$(SUBDIRS):
	$(MAKE) -C $@ all

jbloader: $(OBJECTS) $(SUBDIRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) $(patsubst %.c, %.o, $(wildcard $(patsubst %, %/*.c, $(SUBDIRS)))) -o jbloader
	$(STRIP) jbloader
ifeq ($(ASAN),1)
	$(I_N_T) -add_rpath /cores jbloader
endif
	ldid -S$(SRC)/ent.xml -Icom.apple.jbloader jbloader

xpchook.dylib:
	$(MAKE) -C launchctl/tools LDFLAGS="$(LDFLAGS) $(CFLAGS)" xpchook.dylib
	ldid -S launchctl/tools/xpchook.dylib

%.o: %.c
	$(CC) -c $(CFLAGS) $<

launchctl/%.o: launchctl/%.c
	$(MAKE) -C launchctl $(shell echo $@ | cut -d/ -f2)

create_fakefs_sh.c: create_fakefs.sh
	xxd -iC create_fakefs.sh > create_fakefs_sh.c

.PHONY: all xpchook.dylib $(SUBDIRS)
