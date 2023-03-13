CC = cc
CFLAGS = -O3 -Wall -Wextra -I$(SRC)/dyld_platform_test -I$(SRC)/include
SOURCES =  dyld_platform_ios15.c dyld_platform_ios16.c
SOURCES += dyld_platform_old.c patch_dyld.c plooshfinder.c

all: dyld_platform_test

dyld_platform_test: $(patsubst %.c, $(SRC)/jbinit/%.c, $(SOURCES)) main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) main.c $(patsubst %.c, $(SRC)/jbinit/%.c, $(SOURCES)) -o dyld_platform_test

clean:
	rm -f dyld_platform_test

.PHONY: all
