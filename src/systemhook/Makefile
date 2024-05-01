CFLAGS += -mappletvos-version-min=12.0 -std=gnu17 -fvisibility=hidden
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
LDFLAGS += -install_name /cores/binpack/usr/lib/systemhook.dylib
OBJDIR = obj
C_SRC = $(wildcard *.c)
LIBS = -framework CoreFoundation ../libjailbreak/libjailbreak.a
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))
DYLIB_NAME = systemhook

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: $(DYLIB_NAME).dylib

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(DYLIB_NAME).dylib: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -shared -o $(DYLIB_NAME).dylib
	$(DSYMUTIL) $@
ifneq ($(DEV_BUILD),1)
	$(STRIP) -x $@
endif
	$(LDID) -S $@

clean:
	rm -rf $(DYLIB_NAME).dylib* obj

.PHONY: all clean
