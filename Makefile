ROOT := $(shell pwd)
MACOSX_SYSROOT = $(shell xcrun -sdk macosx --show-sdk-path)
TARGET_SYSROOT = $(shell xcrun -sdk macosx --show-sdk-path)
CC = xcrun -sdk iphoneos clang
OBJC = $(CC)

export ROOT CC OBJC

all: apple-include
	$(MAKE) -C $(ROOT)/src

apple-include:
	mkdir -p apple-include/{bsm,objc,os/internal,sys,firehose,CoreFoundation,FSEvents,IOKit/kext,libkern,kern,arm,{mach/,}machine,CommonCrypto,Security,CoreSymbolication,Kernel/{kern,IOKit,libkern},rpc,rpcsvc,xpc/private,ktrace,mach-o,dispatch}
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
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/stdlib.h > apple-include/stdlib.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/time.h > apple-include/time.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/unistd.h > apple-include/unistd.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/task.h > apple-include/mach/task.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/mach/mach_host.h > apple-include/mach/mach_host.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/ucontext.h > apple-include/ucontext.h
	gsed -E s/'__IOS_PROHIBITED|__TVOS_PROHIBITED|__WATCHOS_PROHIBITED'//g < $(TARGET_SYSROOT)/usr/include/signal.h > apple-include/signal.h
	gsed -E /'__API_UNAVAILABLE'/d < $(TARGET_SYSROOT)/usr/include/pthread.h > apple-include/pthread.h
	gsed -i -E s/'__API_UNAVAILABLE\(.*\)'// apple-include/IOKit/IOKitLib.h

clean:
	$(MAKE) -C $(ROOT)/src clean
	rm -rf apple-include

.PHONY: all clean apple-include
