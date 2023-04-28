SUBDIRS = jbinit jbloader systemhooks

all: $(SUBDIRS)
	@echo $(SUBDIRS)

jbinit:
	$(MAKE) -C $(SRC)/jbinit

jbloader:
	$(MAKE) -C $(SRC)/jbloader

systemhooks:
	$(MAKE) -C $(SRC)/systemhooks jb.dylib injector.dylib

.PHONY: all $(SUBDIRS)
