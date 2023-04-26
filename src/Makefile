SUBDIRS = jbinit jbloader launchd_hook

all: $(SUBDIRS)
	@echo $(SUBDIRS)

jbinit:
	$(MAKE) -C $(SRC)/jbinit

jbloader:
	$(MAKE) -C $(SRC)/jbloader

launchd_hook:
	$(MAKE) -C $(SRC)/launchd_hook jb.dylib injector.dylib

.PHONY: all $(SUBDIRS)
