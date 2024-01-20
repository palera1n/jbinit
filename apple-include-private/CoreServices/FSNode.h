#ifndef CORESERVICES_FSNODE_H
#define CORESERVICES_FSNODE_H
#include <CoreServices/FSNodePropertyProviding.h>
#include <Foundation/Foundation.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvisibility"

@interface FSNode : NSObject <FSNodePropertyProviding, NSCopying, NSSecureCoding> {
    unsigned long long  _cacheExpiration;
    unsigned int  _canUseFileCache;
    unsigned int  _hasReferringAliasNode;
    unsigned int  _isDirectory;
    unsigned int  _isInitialized;
    NSURL * _url;
}

@property (getter=isAVCHDCollection, readonly) bool AVCHDCollection;
@property (getter=isAliasFile, nonatomic, readonly) bool aliasFile;
@property (getter=isBusyDirectory, nonatomic, readonly) bool busyDirectory;
@property (nonatomic, readonly) bool canIssueIO;
@property (readonly, copy) NSString *debugDescription;
@property (readonly, copy) NSString *description;
@property (getter=isDirectory, nonatomic, readonly) bool directory;
@property (getter=isExecutable, nonatomic, readonly) bool executable;
@property (getter=isExecutableModeFile, nonatomic, readonly) bool executableModeFile;
@property (nonatomic, readonly) bool hasHiddenExtension;
@property (nonatomic, readonly) bool hasPackageBit;
@property (readonly) NSUInteger hash;
@property (getter=isHidden, nonatomic, readonly) bool hidden;
@property (getter=isMountTrigger, nonatomic, readonly) bool mountTrigger;
@property (getter=isOnDiskImage, nonatomic, readonly) bool onDiskImage;
@property (getter=isOnLocalVolume, nonatomic, readonly) bool onLocalVolume;
@property (getter=isRegularFile, nonatomic, readonly) bool regularFile;
@property (getter=isResolvable, nonatomic, readonly) bool resolvable;
@property (getter=isSecuredSystemContent, nonatomic, readonly) bool securedSystemContent;
@property (getter=isSideFault, nonatomic, readonly) bool sideFault;
@property (readonly) Class superclass;
@property (getter=isSymbolicLink, nonatomic, readonly) bool symbolicLink;
@property (getter=isVolume, nonatomic, readonly) bool volume;

+ (id)_resolvedNodeFromAliasFile:(id)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
+ (id)_resolvedURLFromAliasFile:(id)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
+ (bool)canAccessURL:(id)arg1 fromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg2 operation:(const char *)arg3;
+ (bool)canAccessURL:(id)arg1 withAuditToken:(const struct { unsigned int x1[8]; }*)arg2 operation:(const char *)arg3;
+ (bool)canReadMetadataOfURL:(id)arg1 fromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (bool)canReadMetadataOfURL:(id)arg1 withAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (bool)canReadURL:(id)arg1 fromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (bool)canReadURL:(id)arg1 withAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (bool)canWriteURL:(id)arg1 fromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (bool)canWriteURL:(id)arg1 withAuditToken:(const struct { unsigned int x1[8]; }*)arg2;
+ (unsigned char)compareBookmarkData:(id)arg1 toBookmarkData:(id)arg2;
+ (bool)getFileSystemRepresentation:(BOOL)arg1 forBookmarkData:(id)arg2;
+ (bool)getVolumeIdentifier:(unsigned long long*)arg1 forBookmarkData:(id)arg2 error:(id*)arg3;
+ (bool)isBookmarkDataFull:(id)arg1;
+ (id)nameForBookmarkData:(id)arg1 error:(id*)arg2;
+ (id)pathForBookmarkData:(id)arg1 error:(id*)arg2;
+ (id)prebootVolumeNode;
+ (id)rootVolumeNode;
+ (bool)supportsSecureCoding;
+ (id)systemDataVolumeNode;
+ (id)userDataVolumeNode;
- (NSURL*)URL;
- (id)bookmarkDataRelativeToNode:(id)arg1 error:(id*)arg2;
- (id)bookmarkDataWithOptions:(unsigned long long)arg1 relativeToNode:(id)arg2 error:(id*)arg3;
- (id)bundleIdentifierWithContext:(struct LSContext { id x1; }*)arg1 error:(id*)arg2;
- (id)bundleIdentifierWithError:(id*)arg1;
- (id)bundleInfoDictionaryWithError:(id*)arg1;
- (bool)canIssueIO;
- (bool)canReadFromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (bool)canReadMetadataFromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (bool)canReadMetadataWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (bool)canReadWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (bool)canWriteFromSandboxWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (bool)canWriteWithAuditToken:(const struct { unsigned int x1[8]; }*)arg1;
- (id)canonical:(bool)arg1 pathWithError:(id*)arg2;
- (id)canonicalPathWithError:(id*)arg1;
- (bool)checkResourceIsReachableAndReturnError:(id*)arg1;
- (id)childNodeWithRelativePath:(id)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
- (bool)childNodeWithRelativePathExists:(id)arg1;
- (void)clearURLPropertyCacheIfStale;
- (struct __CFBundle { }*)copyCFBundleWithError:(id*)arg1;
- (id)copyWithZone:(struct _NSZone { }*)arg1;
- (id)description;
- (id)diskImageURLWithFlags:(unsigned int)arg1 error:(id*)arg2;
- (void)encodeWithCoder:(id)arg1;
- (id)extendedAttributeWithName:(id)arg1 options:(int)arg2 error:(id*)arg3;
- (id)extensionWithError:(id*)arg1;
- (bool)getCachedResourceValueIfPresent:(id*)arg1 forKey:(id)arg2 error:(id*)arg3;
- (bool)getContentModificationDate:(double*)arg1 error:(id*)arg2;
- (bool)getCreationDate:(double*)arg1 error:(id*)arg2;
- (bool)getDeviceNumber:(int*)arg1 error:(id*)arg2;
- (bool)getFileIdentifier:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getFileSystemRepresentation:(BOOL)arg1 error:(id*)arg2;
- (bool)getFinderInfo:(union { unsigned char x1[32]; struct { struct FileInfo { unsigned int x_1_2_1; unsigned int x_1_2_2; unsigned short x_1_2_3; struct Point { short x_4_3_1; short x_4_3_2; } x_1_2_4; unsigned short x_1_2_5; } x_2_1_1; unsigned char x_2_1_2[16]; } x2; struct { struct FolderInfo { struct Rect { short x_1_3_1; short x_1_3_2; short x_1_3_3; short x_1_3_4; } x_1_2_1; unsigned short x_1_2_2; struct Point { short x_3_3_1; short x_3_3_2; } x_1_2_3; unsigned short x_1_2_4; } x_3_1_1; unsigned char x_3_1_2[16]; } x3; }*)arg1 error:(id*)arg2;
- (bool)getHFSType:(unsigned int*)arg1 creator:(unsigned int*)arg2 error:(id*)arg3;
- (bool)getInodeNumber:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getIsDirectory_NoIO:(bool*)arg1;
- (bool)getLength:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getOwnerUID:(unsigned int*)arg1 error:(id*)arg2;
- (bool)getResourceValue:(id*)arg1 forKey:(id)arg2 options:(unsigned char)arg3 error:(id*)arg4;
- (bool)getTemporaryResourceValue:(id*)arg1 forKey:(id)arg2;
- (bool)getVolumeIdentifier:(unsigned long long*)arg1 error:(id*)arg2;
- (bool)getWriterBundleIdentifier:(id*)arg1 error:(id*)arg2;
- (bool)hasHiddenExtension;
- (bool)hasPackageBit;
- (NSUInteger)hash;
- (id)initByResolvingBookmarkData:(id)arg1 options:(unsigned long long)arg2 relativeToNode:(id)arg3 bookmarkDataIsStale:(bool*)arg4 error:(id*)arg5;
- (id)initByResolvingBookmarkData:(id)arg1 relativeToNode:(id)arg2 bookmarkDataIsStale:(bool*)arg3 error:(id*)arg4;
- (id)initTemporaryNodeOnVolume:(id)arg1 flags:(unsigned int)arg2 fileDescriptor:(int*)arg3 error:(id*)arg4;
- (id)initWithCoder:(id)arg1;
- (id)initWithConfigurationString:(int)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
- (id)initWithDirectory:(unsigned long long)arg1 inDomain:(unsigned long long)arg2 lastPathComponent:(id)arg3 createIntermediateDirectories:(bool)arg4 flags:(unsigned int)arg5 error:(id*)arg6;
- (id)initWithFileSystemRepresentation:(const char *)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
- (id)initWithPath:(id)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
- (id)initWithURL:(id)arg1 flags:(unsigned int)arg2 error:(id*)arg3;
- (bool)isAVCHDCollection;
- (bool)isAliasFile;
- (bool)isBusyDirectory;
- (bool)isDirectory;
- (bool)isEqual:(id)arg1;
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
- (id)nameWithError:(id*)arg1;
- (id)pathWithError:(id*)arg1;
- (void)prepareForReuse;
- (id)redactedDescription;
- (id)referringAliasNode;
- (void)removeCachedResourceValueForKey:(id)arg1;
- (id)resolvedNodeWithFlags:(unsigned int)arg1 error:(id*)arg2;
- (bool)setExtendedAttribute:(id)arg1 name:(id)arg2 options:(int)arg3 error:(id*)arg4;
- (bool)setFinderInfo:(const union { unsigned char x1[32]; struct { struct FileInfo { unsigned int x_1_2_1; unsigned int x_1_2_2; unsigned short x_1_2_3; struct Point { short x_4_3_1; short x_4_3_2; } x_1_2_4; unsigned short x_1_2_5; } x_2_1_1; unsigned char x_2_1_2[16]; } x2; struct { struct FolderInfo { struct Rect { short x_1_3_1; short x_1_3_2; short x_1_3_3; short x_1_3_4; } x_1_2_1; unsigned short x_1_2_2; struct Point { short x_3_3_1; short x_3_3_2; } x_1_2_3; unsigned short x_1_2_4; } x_3_1_1; unsigned char x_3_1_2[16]; } x3; }*)arg1 error:(id*)arg2;
- (void)setReferringAliasNode:(id)arg1;
- (bool)setResourceValue:(id)arg1 forKey:(id)arg2 options:(unsigned char)arg3 error:(id*)arg4;
- (void)setTemporaryResourceValue:(id)arg1 forKey:(id)arg2;
- (id)sideFaultResourceValuesWithError:(id*)arg1;
- (id)temporaryDirectoryNodeWithFlags:(unsigned int)arg1 error:(id*)arg2;
- (id)volumeNodeWithFlags:(unsigned int)arg1 error:(id*)arg2;

@end

#pragma clang diagnostic pop
#endif
