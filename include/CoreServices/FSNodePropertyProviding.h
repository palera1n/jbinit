#ifndef CORESERVICES_FSNODEPROPERTYPROVIDING_H
#define CORESERVICES_FSNODEPROPERTYPROVIDING_H
#include <Foundation/Foundation.h>

@protocol FSNodePropertyProviding <NSObject>

@required

- (NSURL *)URL;
- (bool)canIssueIO;
- (NSString *)canonicalPathWithError:(id*)arg1;
- (bool)childNodeWithRelativePathExists:(NSString *)arg1;
- (NSString *)extensionWithError:(id*)arg1;
- (bool)getCachedResourceValueIfPresent:(id*)arg1 forKey:(NSString *)arg2 error:(id*)arg3;
- (bool)getContentModificationDate:(double*)arg1 error:(id*)arg2;
- (bool)getCreationDate:(double*)arg1 error:(id*)arg2;
- (bool)getDeviceNumber:(int*)arg1 error:(id*)arg2;
- (bool)getFileIdentifier:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getFileSystemRepresentation:(BOOL)arg1 error:(id*)arg2;
- (bool)getHFSType:(unsigned int*)arg1 creator:(unsigned int*)arg2 error:(id*)arg3;
- (bool)getInodeNumber:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getLength:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getOwnerUID:(unsigned int*)arg1 error:(id*)arg2;
- (bool)getResourceValue:(id*)arg1 forKey:(NSString *)arg2 options:(unsigned char)arg3 error:(id*)arg4;
- (bool)getVolumeIdentifier:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)hasHiddenExtension;
- (bool)hasPackageBit;
- (bool)isAVCHDCollection;
- (bool)isAliasFile;
- (bool)isBusyDirectory;
- (bool)isDirectory;
- (bool)isExecutable;
- (bool)isExecutableModeFile;
- (bool)isHidden;
- (bool)isMountTrigger;
- (bool)isOnDiskImage;
- (bool)isOnLocalVolume;
- (bool)isRegularFile;
- (bool)isResolvable;
- (bool)isSecuredSystemContent;
- (bool)isSideFault;
- (bool)isSymbolicLink;
- (bool)isVolume;
- (NSString *)nameWithError:(id*)arg1;
- (NSString *)pathWithError:(id*)arg1;
- (NSDictionary *)sideFaultResourceValuesWithError:(id*)arg1;

@end
#endif
