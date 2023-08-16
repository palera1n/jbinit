#ifndef _APFS_H_
#define _APFS_H_

#include <os/base.h>
#include <os/lock.h>
#include <uuid/uuid.h>
#include <device/device_types.h>

#include <APFS/APFSConstants.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

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
APFSContainerGetFreespaceInfo(CFStringRef* container, int(*arg2)(CFStringRef, int, int*));

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
APFSVolumeRole(const char *device, int16_t *role, CFMutableArrayRef *buf);

int
APFSVolumeRoleFind(const char *device, int16_t role, CFMutableArrayRef *buf);

/*
APFSContainerGetResizeProgress
APFSContainerGetMinimalSize
APFSContainerGetSpaceInfo
APFSContainerResize
APFSContainerResizeEx
APFSContainerVolumeGroupAdd
APFSContainerVolumeGroupGet
APFSContainerVolumeGroupGetFirmlinks
APFSContainerVolumeGroupGetSystemAndDataVolumes
APFSContainerVolumeGroupGetVolumes
APFSContainerWriteBurstStats

APFSGetFragmentationHistogram

APFSMakeCompatibleName

APFSStatistics
APFSStatisticsProcessContainer
APFSStreamCreateEstimateProgress
APFSStreamCreateFinish
APFSStreamCreatePrepare
APFSStreamCreateRead
APFSStreamFingerprintFinish
APFSStreamFingerprintPrepare
APFSStreamFingerprintWrite
APFSStreamRestoreFinish
APFSStreamRestorePrepare
APFSStreamRestoreWrite

APFSUniquifyName

APFSVolumeAddHints
APFSVolumeAddHintsWithOptions
APFSVolumeAddUnlockRecords
APFSVolumeAddUnlockRecordsWithOptions
APFSVolumeBindNewKEKToVEK
APFSVolumeBindNewKEKToVEKWithOptions
APFSVolumeDisableFileVault
APFSVolumeDisableFileVaultWithOptions
APFSVolumeEnableFilevault
APFSVolumeEnableFilevaultWithOptions
APFSVolumeEnableUserProtection
APFSVolumeEnableUserProtectionWithOptions
APFSVolumeEscrowVEK
APFSVolumeGeneratePersonalRecoveryKey
APFSVolumeGetDefrag
APFSVolumeGetHint
APFSVolumeGetSiDPState
APFSVolumeGetSpaceInfo
APFSVolumeGetUnlockRecord
APFSVolumeGetVEKState
APFSVolumeGetWVEK
APFSVolumeInfoForUnmountedDisk
APFSVolumeListUUIDsOfUnlockRecords
APFSVolumePauseCrypto
APFSVolumePayloadGet
APFSVolumePayloadSet
APFSVolumeQueryCryptoProgress
APFSVolumeRemoveHint
APFSVolumeRemoveUnlockRecord
APFSVolumeResetUnlockRecord
APFSVolumeResetUnlockRecordWithOptions
APFSVolumeResumeCrypto

APFSVolumeSetDefrag
APFSVolumeSetHint
APFSVolumeSetUnlockRecord
APFSVolumeUnlockAnyUnlockRecord
APFSVolumeUnlockAnyUnlockRecordWithOptions
APFSVolumeUnlockGetUnlockRecordState
APFSVolumeUnlockUnlockRecord
APFSVolumeUnlockUnlockRecordWithOptions
APFSVolumeUpdateBounds
APFSVolumeUpdateUnlockRecord
APFSVolumeUpdateUnlockRecordWithOptions
APFSVolumeVerifyUnlockRecord
APFSVolumeVerifyUnlockRecordWithOptions
*/

__END_DECLS

#endif
