OBJECTS += $(patsubst %.c, %.o, $(wildcard *.c))

all: $(OBJECTS)

%.o: %.c
	$(CC) -c -I.. $(CFLAGS) $<

.PHONY: all
