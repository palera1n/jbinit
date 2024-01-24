CFLAGS += -miphoneos-version-min=12.0 -std=gnu17
LDFLAGS += -Wl,-dead_strip -Wno-unused-command-line-argument
OBJDIR = obj
C_SRC = $(wildcard *.c)
LIBS = -framework CoreFoundation
OBJS = $(patsubst %,$(OBJDIR)/%,$(C_SRC:.c=.c.o))

ifeq ($(ASAN),1)
CFLAGS += -fsanitize=address,undefined
LDFLAGS += -rpath /cores
endif

all: libjailbreak.a

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

libjailbreak.a: $(OBJS)
	$(AR) cr libjailbreak.a $(OBJS);

clean:
	rm -rf libjailbreak.a* obj

.PHONY: all clean
