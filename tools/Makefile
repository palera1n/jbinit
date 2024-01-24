CC = $(CC_FOR_BUILD)
CXX = $(CXX_FOR_BUILD)
LD = $(LD_FOR_BUILD)
CFLAGS = $(CFLAGS_FOR_BUILD)
CXXFLAGS = $(CXXFLAGS_FOR_BUILD)
LDFLAGS = $(LDFLAGS_FOR_BUILD)

export CC CXX LD CFLAGS CXXFLAGS LDFLAGS

libdmg-hfsplus:
	printf '.gitignore\nbuild\n' > "$@/.gitignore"
	mkdir -p "$@/build"
	cd "$@/build" && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$(CC)" -DCMAKE_C_FLAGS="$(CFLAGS)" "$(ROOT)/tools/$@"
	$(MAKE) -C "$(ROOT)/tools/$@/build"

clean:
	rm -rf libdmg-hfsplus/build

.PHONY: libdmg-hfsplus
