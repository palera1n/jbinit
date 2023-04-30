CC = cc
CFLAGS = -O3 -Wall -Wextra -I$(SRC)/dyld_platform_test -I$(SRC)/jbinit/patch_dyld -I.
SOURCES = $(patsubst %.c, %.o, $(wildcard patch_dyld/*.c)) patch_dyld_bridge.c

all: dyld_platform_test

dyld_platform_test: $(SOURCES) main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) main.c $(SOURCES) -o dyld_platform_test

clean:
	rm -f dyld_platform_test

.PHONY: all
