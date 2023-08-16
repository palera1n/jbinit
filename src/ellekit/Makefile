ELLEKIT = ellekit_build/Build/Products/Release-iphoneos/libellekit.dylib

all: $(ELLEKIT)

$(ELLEKIT): ../../.git/modules/src/rootlesshooks/ellekit/refs/heads/main
	cd ellekit; \
		CC="" CXX="" LD="" xcodebuild build -arch arm64 -sdk iphoneos -scheme ellekit -configuration Release -derivedDataPath ../ellekit_build \
		IPHONEOS_DEPLOYMENT_TARGET=14.0 \
		CODE_SIGNING_ALLOWED=NO \
		LD_DYLIB_INSTALL_NAME="@rpath/CydiaSubstrate.framework/CydiaSubstrate"
	$(LDID) -S $(ELLEKIT);

clean:
	rm -rf ellekit_build obj

.PHONY: all clean
