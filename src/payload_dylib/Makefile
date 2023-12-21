CFLAGS += -miphoneos-version-min=12.0 -std=gnu17
CFLAGS += -isystem $(ROOT)/apple-include -I$(ROOT)/include
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
LDFLAGS += -install_name /cores/payload.dylib
OBJDIR = obj
C_SRC = $(wildcard *.c)
LIBS = -framework CoreFoundation ../libjailbreak/libjailbreak.a
LIBS += -framework IOKit -framework Foundation ../libs/IOMobileFramebuffer.tbd
LIBS += -framework CoreGraphics -framework ImageIO -framework IOSurface
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: payload.dylib

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

payload.dylib: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -shared -o $@
	$(DSYMUTIL) $@
	$(STRIP) -x $@
	$(LDID) -S $@

clean:
	rm -rf payload.dylib* obj

.PHONY: all clean
