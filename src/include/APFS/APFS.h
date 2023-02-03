#ifndef _APFS_H_
#define _APFS_H_

#include <os/base.h>
#include <os/lock.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <APFS/APFSConstants.h>

__BEGIN_DECLS

int
APFSCancelContainerResize(os_unfair_lock_t lock);

int
APFSContainerDefrag(const char *bsdName);

int
APFSContainerEFIEmbed(const char *bsdName, const char **Ptr, size_t PtrSize);

int
APFSContainerEFIGetVersion(const char *bsdName, const char **Ptr, size_t PtrSize, void *outputStruct);

int
APFSContainerGetBootDevice(CFStringRef *container);

int
APFSContainerGetDefrag(const char *bsdName, int *buf);

int
APFSContainerGetFreeExtentHistogram(io_service_t device, CFDictionaryRef dict);

int
APFSContainerGetMaxVolumeCount(const char *device, CFIndex *buf);

int
APFSContainerGetMinimalSize(const char *device, CFIndex *buf);

int
APFSContainerMigrateMediaKeys(const char *container);

int
APFSContainerSetDefrag(const char *bsdName, int defrag);

int
APFSContainerStitchVolumeGroup(const char *bsdName);

int
APFSContainerVolumeGroupRemove(const char *bsdName, uuid_t uuid);

int
APFSContainerVolumeGroupSyncUnlockRecords(const char *bsdName, uuid_t uuid);

int
APFSContainerWipeVolumeKeys(const char *bsdName);

int
APFSExtendedSpaceInfo(const char *device, CFDictionaryRef dict);

int
APFSGetVolumeGroupID(const char *device, uuid_t uuid);

int
APFSVolumeCreate(const char *device, CFDictionaryRef dict);

int
APFSVolumeCreateForMSU(const char *device, CFDictionaryRef dict);

int
APFSVolumeDelete(const char *device);

int
APFSVolumeRole(const char *device, short *role, CFMutableArrayRef *buf);

int
APFSVolumeRoleFind(const char *device, short role, CFMutableArrayRef *buf);

__END_DECLS

#endif