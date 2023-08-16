SRC = $(wildcard src/*.c) $(wildcard src/formats/*.c) $(wildcard src/asm/*.c)
OBJDIR = obj
OBJS = $(patsubst src/%,$(OBJDIR)/%,$(SRC:.c=.o))
DIRS := $(OBJDIR) $(OBJDIR)/formats $(OBJDIR)/asm

all: $(DIRS) $(OBJS)

$(DIRS):
	@mkdir -p $@

clean:
	@rm -rf obj

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean
