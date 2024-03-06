ROOT := $(shell pwd)
SHELL = /usr/bin/env bash
UNAME != uname -s
CC_FOR_BUILD ?= cc
FAKEROOT = fakeroot
PATH += ":/sbin:/usr/sbin"
ifeq ($(UNAME),Darwin)
PRODUCT_NAME != sw_vers -productName
NEWFS_HFS = newfs_hfs
MAC != echo "$(PRODUCT_NAME)" | grep -qi mac && echo 1
else
NEWFS_HFS = mkfs.hfsplus
endif
LDID = ldid
DSYMUTIL = dsymutil
ifeq ($(UNAME),Darwin)
VTOOL = vtool
STRIP = strip
AR = ar
endif
ifeq ($(MAC),1)
MACOSX_SYSROOT != xcrun -sdk macosx --show-sdk-path
TARGET_SYSROOT != xcrun -sdk appletvos --show-sdk-path
CC != xcrun --find clang
else ifeq ($(UNAME),Darwin)
CC = clang
MACOSX_SYSROOT ?= /usr/share/SDKs/MacOSX.sdk
TARGET_SYSROOT ?= /usr/share/SDKs/AppleTVOS.sdk
else
VTOOL = cctools-vtool
STRIP = cctools-strip
AR = cctools-ar
CC = clang
CFLAGS += -target arm64-apple-tvos
LD != command -v ld64
LDFLAGS += "-fuse-ld=$(LD)"
MACOSX_SYSROOT ?= $(HOME)/cctools/SDKs/MacOSX.sdk
TARGET_SYSROOT ?= $(HOME)/cctools/SDKs/AppleTVOS.sdk
endif
CFLAGS += -isystem $(ROOT)/apple-include -I$(ROOT)/include -isysroot $(TARGET_SYSROOT)
OBJC = $(CC)
HFSPLUS += $(ROOT)/tools/libdmg-hfsplus/build/hfs/hfsplus
DMG += $(ROOT)/tools/libdmg-hfsplus/build/dmg/dmg

ifeq ($(shell command -v gsed > /dev/null && echo 1),1)
SED = gsed
else
SED = sed
endif

SUBDIRS = fakedyld rootlesshooks payload_dylib payload systemhook rootfulhooks universalhooks mount_cores ellekit

export ROOT CC OBJC CFLAGS CC_FOR_BUILD HFSPLUS DMG NEWFS_HFS MAC UNAME SED SHELL LDFLAGS VTOOL STRIP DSYMUTIL LDID AR SUBDIRS

all: binaries tools
	$(MAKE) -C $(ROOT)/src ramdisk.dmg binpack.dmg

binaries: apple-include
	$(MAKE) -C $(ROOT)/src $(patsubst %, %-all, $(SUBDIRS))

apple-include: apple-include-private/**
	mkdir -p apple-include/{bsm,objc,os/internal,sys,firehose,CoreFoundation,FSEvents,IOSurface,IOKit/kext,libkern,kern,arm,{mach/,}machine,CommonCrypto,Security,CoreSymbolication,Kernel/{kern,IOKit,libkern},rpc,rpcsvc,xpc/private,ktrace,mach-o,dispatch}
	cp -af $(MACOSX_SYSROOT)/usr/include/{arpa,bsm,hfs,net,xpc,netinet,servers,timeconv.h,launch.h} apple-include
	cp -af $(MACOSX_SYSROOT)/usr/include/objc/objc-runtime.h apple-include/objc
	cp -af $(MACOSX_SYSROOT)/usr/include/libkern/{OSDebug.h,OSKextLib.h,OSReturn.h,OSThermalNotification.h,OSTypes.h,machine} apple-include/libkern
	cp -af $(MACOSX_SYSROOT)/usr/include/kern apple-include
	cp -af $(MACOSX_SYSROOT)/usr/include/sys/{tty*,ptrace,kern*,random,reboot,user,vnode,disk,vmmeter,conf}.h apple-include/sys
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/Kernel.framework/Versions/Current/Headers/sys/disklabel.h apple-include/sys
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/IOKit.framework/Headers/{AppleConvergedIPCKeys.h,IOBSD.h,IOCFBundle.h,IOCFPlugIn.h,IOCFURLAccess.h,IOKitServer.h,IORPC.h,IOSharedLock.h,IOUserServer.h,audio,avc,firewire,graphics,hid,hidsystem,i2c,iokitmig.h,kext,ndrvsupport,network,ps,pwr_mgt,sbp2,scsi,serial,storage,stream,usb,video} apple-include/IOKit
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/Security.framework/Headers/{mds_schema,oidsalg,SecKeychainSearch,certextensions,Authorization,eisl,SecDigestTransform,SecKeychainItem,oidscrl,cssmcspi,CSCommon,cssmaci,SecCode,CMSDecoder,oidscert,SecRequirement,AuthSession,SecReadTransform,oids,cssmconfig,cssmkrapi,SecPolicySearch,SecAccess,cssmtpi,SecACL,SecEncryptTransform,cssmapi,cssmcli,mds,x509defs,oidsbase,SecSignVerifyTransform,cssmspi,cssmkrspi,SecTask,cssmdli,SecAsn1Coder,cssm,SecTrustedApplication,SecCodeHost,SecCustomTransform,oidsattr,SecIdentitySearch,cssmtype,SecAsn1Types,emmtype,SecTransform,SecTrustSettings,SecStaticCode,emmspi,SecTransformReadTransform,SecKeychain,SecDecodeTransform,CodeSigning,AuthorizationPlugin,cssmerr,AuthorizationTags,CMSEncoder,SecEncodeTransform,SecureDownload,SecAsn1Templates,AuthorizationDB,SecCertificateOIDs,cssmapple}.h apple-include/Security
	cp -af $(MACOSX_SYSROOT)/usr/include/{ar,bootstrap,launch,libc,libcharset,localcharset,nlist,NSSystemDirectories,tzfile,vproc}.h apple-include
	cp -af $(MACOSX_SYSROOT)/usr/include/mach/{*.defs,{mach_vm,shared_region}.h} apple-include/mach
	cp -af $(MACOSX_SYSROOT)/usr/include/mach/machine/*.defs apple-include/mach/machine
	cp -af $(MACOSX_SYSROOT)/usr/include/rpc/pmap_clnt.h apple-include/rpc
	cp -af $(MACOSX_SYSROOT)/usr/include/rpcsvc/yp{_prot,clnt}.h apple-include/rpcsvc
	cp -af $(TARGET_SYSROOT)/usr/include/mach/machine/thread_state.h apple-include/mach/machine
	cp -af $(TARGET_SYSROOT)/usr/include/mach/arm apple-include/mach
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/IOKit.framework/Headers/* apple-include/IOKit
	cp -af $(MACOSX_SYSROOT)/System/Library/Frameworks/IOSurface.framework/Headers/* apple-include/IOSurface
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/stdlib.h > apple-include/stdlib.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/time.h > apple-include/time.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/unistd.h > apple-include/unistd.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/task.h > apple-include/mach/task.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/mach_host.h > apple-include/mach/mach_host.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/mach.h > apple-include/mach/mach.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/message.h > apple-include/mach/message.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/ucontext.h > apple-include/ucontext.h
	$(SED) -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/signal.h > apple-include/signal.h
	$(SED) -E s/'__API_UNAVAILABLE(.*)'/';'/g < $(TARGET_SYSROOT)/usr/include/spawn.h > apple-include/spawn.h
	$(SED) 's/#ifndef __OPEN_SOURCE__/#if 1\n#if defined(__has_feature) \&\& defined(__has_attribute)\n#if __has_attribute(availability)\n#define __API_AVAILABLE_PLATFORM_bridgeos(x) bridgeos,introduced=x\n#define __API_DEPRECATED_PLATFORM_bridgeos(x,y) bridgeos,introduced=x,deprecated=y\n#define __API_UNAVAILABLE_PLATFORM_bridgeos bridgeos,unavailable\n#endif\n#endif/g' < $(TARGET_SYSROOT)/usr/include/AvailabilityInternal.h > apple-include/AvailabilityInternal.h
	$(SED) -E /'__API_UNAVAILABLE'/d < $(TARGET_SYSROOT)/usr/include/pthread.h > apple-include/pthread.h
	@if [ -f $(TARGET_SYSROOT)/System/Library/Frameworks/CoreFoundation.framework/Headers/CFUserNotification.h ]; then $(SED) -E 's/API_UNAVAILABLE\(ios, watchos, tvos\)//g' < $(TARGET_SYSROOT)/System/Library/Frameworks/CoreFoundation.framework/Headers/CFUserNotification.h > apple-include/CoreFoundation/CFUserNotification.h; fi
	$(SED) -i -E s/'__API_UNAVAILABLE\(.*\)'// apple-include/IOKit/IOKitLib.h
	$(SED) -E -e s/'API_UNAVAILABLE\(.*\)'// -e 's/API_AVAILABLE\(macos\(10\.7\)\)/__OSX_AVAILABLE_STARTING\(__MAC_10_7, __IPHONE_5_0\)/g' < $(MACOSX_SYSROOT)/usr/include/xpc/connection.h > apple-include/xpc/connection.h
	cp -a apple-include-private/. apple-include

clean:
	$(MAKE) -C $(ROOT)/src clean
	rm -rf apple-include

distclean: clean
	$(MAKE) -C $(ROOT)/tools clean
	$(MAKE) -C $(ROOT)/src distclean

tools:
	$(MAKE) -C $(ROOT)/tools

patch_dyld-test:
	$(MAKE) -C $(ROOT)/tools patch_dyld-test

.PHONY: all clean apple-include tools patch_dyld-test
