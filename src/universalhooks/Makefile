CFLAGS += -miphoneos-version-min=14.0 -std=gnu17
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
OBJDIR = obj
C_SRC = $(wildcard *.c)
OBJC_SRC = $(wildcard *.m)
OBJCFLAGS = -fobjc-arc
LIBS = -lobjc -framework IOKit -framework CoreFoundation ../libs/libellekit.tbd
LIBS += -framework Security
OBJS = $(patsubst %,$(OBJDIR)/%,$(OBJC_SRC:.m=.m.o) $(C_SRC:.c=.c.o))

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: universalhooks.dylib

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.m.o: %.m $(OBJDIR)
	$(OBJC) $(CFLAGS) $(OBJCFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

universalhooks.dylib: $(OBJS) $(ELLEKIT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -shared \
		-rpath /Library/Frameworks \
		-rpath /usr/lib \
		-rpath /cores/binpack/Library/Frameworks \
		-rpath /cores/binpack/usr/lib \
		-install_name /cores/binpack/usr/lib/universalhooks.dylib \
		-o $@
	$(DSYMUTIL) $@
	$(STRIP) -x $@
	$(LDID) -S $@

clean:
	rm -rf universalhooks.dylib* universalhooks.dylib.dSYM obj

.PHONY: all clean

