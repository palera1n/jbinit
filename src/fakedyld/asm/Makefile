OBJDIR = obj
SRC = $(wildcard *.S)
OBJS = $(patsubst %,$(OBJDIR)/%,$(SRC:.S=.o))

all: $(OBJS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.S $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj

.PHONY: all clean
