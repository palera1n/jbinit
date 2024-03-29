CFLAGS += -mappletvos-version-min=12.0 -std=gnu17 -fvisibility=hidden
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
CXXFLAGS = -mappletvos-version-min=12.0 -fvisibility=hidden
CXXFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
OBJDIR = obj
C_SRC = $(wildcard *.c)
OBJC_SRC = $(wildcard *.m)
LIBS += -framework Foundation -lobjc ./dobby/build/libdobby.a -lc++ -shared
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o)) $(patsubst %,$(OBJDIR)/%,$(OBJC_SRC:.m=.m.o))

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: bridgehook.dylib

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.m.o: %.m $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

dobby/build/libdobby.a:
	mkdir -p "dobby/build"
	cd "dobby/build" && cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_COMPILER="$(CC)" \
		-DCMAKE_CXX_COMPILER="$(CXX)" \
		-DCMAKE_C_FLAGS="$(CFLAGS)" \
		-DCMAKE_CXX_FLAGS="$(CXXFLAGS)" \
		-DCMAKE_CROSSCOMPILING=true \
		-DCMAKE_EXE_LINKER_FLAGS="-mappletvos-version-min=12.0" \
		-DCMAKE_SYSTEM_NAME=Darwin \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
		-DCMAKE_SYSTEM_PROCESSOR=arm64 \
		-DDOBBY_BUILD_SILICON=ON \
		-DCMAKE_OSX_SYSROOT="$(TARGET_SYSROOT)" \
		-DCMAKE_OSX_ARCHITECTURES="arm64" \
		"$(ROOT)/src/bridgehook/dobby"
	$(MAKE) -C "dobby/build" dobby_static

bridgehook.dylib: $(OBJS) dobby/build/libdobby.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(OBJS) -o $@
	$(DSYMUTIL) $@
ifneq ($(DEV_BUILD),1)
	$(STRIP) -x $@
endif
	$(LDID) -S $@

clean:
	rm -rf bridgehook.dylib* bridgehook.dylib.dSYM obj dobby/build

.PHONY: all clean
