CFLAGS += -miphoneos-version-min=12.0 -std=gnu17 -U__nonnull -DMOUNT_CORES
CFLAGS += -isystem $(ROOT)/apple-include
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
SRC = $(wildcard *.c)
OBJDIR = obj
OBJS = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.o))
LIBS = -framework CoreFoundation -framework IOKit
PLATFORMS = 2 3 5

all: $(patsubst %, mount_cores.%, $(PLATFORMS))

OBJS = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.o))

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

mount_cores: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(DSYMUTIL) $@
ifeq ($(DEV_BUILD),1)
	$(STRIP) $@
endif

mount_cores.%: mount_cores entitlements.xml
	$(VTOOL) -remove-build-version 2 -output $@ $<
	if [ "$@" = "mount_cores.5" ]; then \
		os=3; \
	else os=12; \
	fi; \
	$(VTOOL) -set-build-version $$(echo $@ | cut -d. -f2) $$os $$os -replace -output $@ $@
	$(LDID) -Sentitlements.xml $@

clean:
	rm -fr mount_cores* obj
