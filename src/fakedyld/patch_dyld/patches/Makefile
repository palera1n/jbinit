SUBDIRS = platform

$(patsubst %, %-all, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1)

$(patsubst %, %-clean, $(SUBDIRS)):
	$(MAKE) -C $(shell echo $@ | cut -d- -f1) clean

all: $(patsubst %, %-all, $(SUBDIRS))

clean: $(patsubst %, %-clean, $(SUBDIRS))

.PHONY: all $(patsubst %, %-all, $(SUBDIRS)) $(patsubst %, %-clean, $(SUBDIRS))
