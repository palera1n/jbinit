SUBDIRS = loader p1ctl jailbreakd
CFLAGS += -mappletvos-version-min=12.0 -std=gnu17
CFLAGS += -Wno-nullability-completeness
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
LDFLAGS += -Wl,-sectcreate,__TEXT,__info_plist,Info.plist
OBJDIR = obj
C_SRC = $(wildcard *.c)
LIBS = -framework CoreFoundation ../libs/IOKit.tbd -framework Foundation
LIBS += -framework Security ../libs/IOMobileFramebuffer.tbd
LIBS += -framework CoreGraphics -framework ImageIO -framework IOSurface
LIBS += ../libs/APFS.tbd ../libjailbreak/libjailbreak.a
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

BOOTSCREEND_SRC = bootscreend.c
BOOTSCREEND_CFLAGS = -DTESTMAIN
BOOTSCREEND_LIBS = ../libs/IOKit.tbd ../libs/IOMobileFramebuffer.tbd
BOOTSCREEND_LIBS = -framework CoreFoundation ../libjailbreak/libjailbreak.a

LAUNCHCTL_OBJ = bootstrap.o xpc_helper.o load.o

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

export CFLAGS LDFLAGS

all: $(patsubst %, %-all, $(SUBDIRS)) payload bootscreend

$(OBJDIR):
	@mkdir -p $(OBJDIR) launchctl/$(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(patsubst %.o, launchctl/%.o, $(LAUNCHCTL_OBJ)): $(patsubst %.o, launchctl/%.c, $(LAUNCHCTL_OBJ))
	CFLAGS="$(CFLAGS) -Wno-unused-parameter" $(MAKE) -C launchctl $(shell echo $@ | cut -d/ -f2)

payload: $(patsubst %, %-all, $(SUBDIRS)) $(OBJS) $(patsubst %.o, launchctl/%.o, $(LAUNCHCTL_OBJ))
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(patsubst %.o, launchctl/%.o, $(LAUNCHCTL_OBJ)) $$(find . -name '*.o' | grep 'obj/.*o') -o payload
	$(DSYMUTIL) payload
ifneq ($(DEV_BUILD),1)
	$(STRIP) payload
endif
	$(LDID) -Sentitlements.xml -Icom.apple.payload payload

bootscreend: $(BOOTSCREEND_SRC)
	$(CC) $(CFLAGS) $(BOOTSCREEND_CFLAGS) $(LDFLAGS) $(LIBS) $^ -o $@
	$(LDID) -Sentitlements.xml -Icom.apple.bootscreend bootscreend

$(patsubst %, %-all, $(SUBDIRS)):
	$(MAKE) -C $$(echo $@ | cut -d- -f1)

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $$(echo $@ | cut -d- -f1) clean

clean: $(patsubst %, %-clean, $(SUBDIRS))
	rm -rf payload payload.dSYM $(OBJDIR) launchctl/*.o

.PHONY: all $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS))
