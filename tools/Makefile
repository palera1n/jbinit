CC = $(CC_FOR_BUILD)
CXX = $(CXX_FOR_BUILD)
LD = $(LD_FOR_BUILD)
CFLAGS = $(CFLAGS_FOR_BUILD)
CXXFLAGS = $(CXXFLAGS_FOR_BUILD)
LDFLAGS = $(LDFLAGS_FOR_BUILD)

export CC CXX LD CFLAGS CXXFLAGS LDFLAGS

all: libdmg-hfsplus

libdmg-hfsplus/build/Makefile:
	printf '.gitignore\nbuild\n' > "libdmg-hfsplus/.gitignore"
	mkdir -p "libdmg-hfsplus/build"
	cd "libdmg-hfsplus/build" && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$(CC)" -DCMAKE_C_FLAGS="$(CFLAGS)" "$(ROOT)/tools/libdmg-hfsplus"

libdmg-hfsplus/build/hfs/hfsplus: libdmg-hfsplus/build/Makefile
	$(MAKE) -C "$(ROOT)/tools/libdmg-hfsplus/build" hfsplus

libdmg-hfsplus/build/dmg/dmg: libdmg-hfsplus/build/Makefile
	$(MAKE) -C "$(ROOT)/tools/libdmg-hfsplus/build" dmg-bin

libdmg-hfsplus: libdmg-hfsplus/build/hfs/hfsplus libdmg-hfsplus/build/dmg/dmg

clean:
	rm -rf libdmg-hfsplus/build

.PHONY: libdmg-hfsplus clean all
