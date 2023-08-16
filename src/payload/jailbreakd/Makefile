OBJDIR = obj
C_SRC = $(wildcard *.c)
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

all: $(OBJS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj

.PHONY: all clean
