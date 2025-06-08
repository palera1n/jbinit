ELLEKIT = ellekit_build/Build/Products/Release-iphoneos/libellekit.dylib
ELLEKIT_PLATFORM = iphoneos

all: $(ELLEKIT)

$(ELLEKIT):
ifeq ($(MAC),1)
	cd ellekit; \
		CC="" CXX="" LD="" xcodebuild build -arch arm64 -sdk $(ELLEKIT_PLATFORM) -scheme ellekit -configuration Release -derivedDataPath ../ellekit_build \
		APPLETVOS_DEPLOYMENT_TARGET=14.0 \
		CODE_SIGNING_ALLOWED=NO \
		LD_DYLIB_INSTALL_NAME="@rpath/CydiaSubstrate.framework/CydiaSubstrate"
ifneq ($(DEV_BUILD),1)
	$(STRIP) -x $(ELLEKIT)
endif
	$(LDID) -S $(ELLEKIT);
else
	@echo "ElleKit cannot be built on this machine"
	mkdir -p ellekit_build/Build/Products/Release-$(ELLEKIT_PLATFORM)
	cp libellekit.dylib $(ELLEKIT)
endif
clean:
	@echo "distclean to clean ellekit"

distclean:
	rm -rf ellekit_build obj

.PHONY: all clean distclean
