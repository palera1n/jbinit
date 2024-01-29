C_SRC = main.c $(shell find $(ROOT)/src/fakedyld/patch_dyld -name '*.c')
OBJDIR = obj
CFLAGS = -Iinclude -I$(ROOT)/include/plooshfinder -I$(ROOT)/include
CFLAGS += -flto=full
LDFLAGS = -Wl,-dead_strip
EXE = patch_dyld-test

OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.o))

all: $(EXE)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/$(shell basename %.o): %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $(OBJDIR)/$(shell basename $@) $<

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $(patsubst %,$(OBJDIR)/%,$(shell basename $(OBJS))) -o patch_dyld-test

clean:
	rm -rf $(EXE) $(OBJDIR)

.PHONY: all clean
