CFLAGS += -miphoneos-version-min=14.0 -std=gnu17
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
OBJDIR = obj
C_SRC = $(wildcard *.c)
OBJC_SRC = $(wildcard *.m)
OBJCFLAGS = -fobjc-arc 
LIBS = -lobjc -framework Foundation -framework CoreFoundation ../libs/libellekit.tbd
OBJS = $(patsubst %,$(OBJDIR)/%,$(OBJC_SRC:.m=.m.o) $(C_SRC:.c=.c.o))

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: rootlesshooks.dylib

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.m.o: %.m $(OBJDIR)
	$(OBJC) $(CFLAGS) $(OBJCFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

rootlesshooks.dylib: $(OBJS) $(ELLEKIT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -shared \
		-rpath /var/jb/Library/Frameworks \
		-rpath /var/jb/usr/lib \
		-rpath /cores/binpack/Library/Frameworks \
		-rpath /cores/binpack/usr/lib \
		-install_name /cores/binpack/usr/lib/rootlesshooks.dylib \
		-o $@
	$(DSYMUTIL) $@
	$(STRIP) -x $@
	$(LDID) -S $@

clean:
	rm -rf rootlesshooks.dylib* rootlesshooks.dylib.dSYM obj

.PHONY: all clean
