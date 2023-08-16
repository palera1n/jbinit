OBJDIR = obj
SRC = $(wildcard *.c)
OBJS = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.o))

all: $(OBJS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj

.PHONY: all clean
