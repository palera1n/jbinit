OBJDIR = obj
SUBDIRS = plooshfinder patches
SRC = $(wildcard *.c)
OBJS = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.o))
CFLAGS += -I$(ROOT)/include/plooshfinder

all: $(OBJS) $(patsubst %, %-all, $(SUBDIRS))

$(patsubst %, %-all, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1)

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1) clean

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean: $(patsubst %, %-clean, $(SUBDIRS))
	rm -rf obj

.PHONY: all clean $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS))
