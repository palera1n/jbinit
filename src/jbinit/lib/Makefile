OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c))

all: $(OBJECTS)

%.o: %.c
	$(CC) -c -nostdlibinc -I.. $(CFLAGS) $<

.PHONY: all
