CC = cc
CFLAGS = -O3 -Wall -Wextra -I$(SRC)/dyld_platform_test -I$(SRC)/jbinit/patch_dyld
SOURCES =  ios15.c ios16.c macho.c old.c utils.c plooshfinder.c

all: dyld_platform_test

dyld_platform_test: $(patsubst %.c, $(SRC)/jbinit/patch_dyld/%.c, $(SOURCES)) main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) main.c $(patsubst %.c, $(SRC)/jbinit/patch_dyld/%.c, $(SOURCES)) -o dyld_platform_test

clean:
	rm -f dyld_platform_test

.PHONY: all
