OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c)) create_fakefs_sh.o

all: $(OBJECTS)

create_fakefs_sh.c: create_fakefs.sh
	xxd -iC create_fakefs.sh > create_fakefs_sh.c

%.o: %.c
	$(CC) -c -I.. $(CFLAGS) $<

.PHONY: all
