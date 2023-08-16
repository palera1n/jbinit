#ifndef _APFSCONSTANTS_H_
#define _APFSCONSTANTS_H_

#include <CoreFoundation/CoreFoundation.h>

#define apple_apfs 0x1200

#define APFS_BUNDLE_ID "com.apple.filesystems.apfs"
#define APFS_CONTAINER_OBJECT "AppleAPFSContainer"
#define APFS_VOLUME_OBJECT "AppleAPFSVolume"

#define APFS_VOL_ROLE_NONE		0x0000
#define APFS_VOL_ROLE_SYSTEM		0x0001
#define APFS_VOL_ROLE_USER		0x0002
#define APFS_VOL_ROLE_RECOVERY		0x0004
#define APFS_VOL_ROLE_VM		0x0008
#define APFS_VOL_ROLE_PREBOOT		0x0010
#define APFS_VOL_ROLE_INSTALLER		0x0020
#define APFS_VOL_ROLE_DATA		0x0040
#define APFS_VOL_ROLE_BASEBAND		0x0080
#define APFS_VOL_ROLE_RESERVED_200	0x0200

#define APFS_VOL_ROLES_VALID_MASK	(APFS_VOL_ROLE_SYSTEM \
					| APFS_VOL_ROLE_USER \
					| APFS_VOL_ROLE_RECOVERY \
					| APFS_VOL_ROLE_VM \
					| APFS_VOL_ROLE_PREBOOT \
					| APFS_VOL_ROLE_INSTALLER \
					| APFS_VOL_ROLE_DATA \
					| APFS_VOL_ROLE_BASEBAND \
					| APFS_VOL_ROLE_RESERVED_200)

#define EDT_OS_ENV_MAIN 1
#define EDT_OS_ENV_OTHER 2
#define EDT_OS_ENV_DIAGS 3

#define EDTVolumeFSType			"apfs"
#define EDTVolumePropertySize		(2 << 4)
#define EDTVolumePropertyMaxSize	(2 << 7) // Guessed

#define kAPFSStatusKey "Status"
#define kAPFSRoleKey "Role"
#define kAPFSVolumeRoleSystem "System"
#define kAPFSVolGroupUUIDKey "VolGroupUUID"

#define kEDTFilesystemEntry "IODeviceTree:/filesystems/fstab"
#define kEDTOSEnvironment CFSTR("os_env_type")

enum {
    kAPFSXSubType    = 0,    /* APFS Case-sensitive */
    kAPFSSubType     = 1     /* APFS Case-insensitive */
};

extern CFStringRef kAPFSContainerBlocksizeKey;
extern CFStringRef kAPFSContainerExtentAddressKey;
extern CFStringRef kAPFSContainerExtentLengthKey;
extern CFStringRef kAPFSContainerExtentsListKey;
extern CFStringRef kAPFSContainerFSTypeKey;
extern CFStringRef kAPFSContainerSizeKey;
extern CFStringRef kAPFSContainerTidemarkKey;
extern CFStringRef kAPFSStreamCreateEmbedCRC;
extern CFStringRef kAPFSStreamCreateReadAlignment;
extern CFStringRef kAPFSVolumeCaseSensitiveKey;
extern CFStringRef kAPFSVolumeEffaceableKey;
extern CFStringRef kAPFSVolumeEncryptedKey;
extern CFStringRef kAPFSVolumeEncryptedACMKey;
extern CFStringRef kAPFSVolumeFSIndexKey;
extern CFStringRef APFSVolumeGroupSiblingFSIndexKey;
extern CFStringRef kAPFSVolumeNameKey;
extern CFStringRef kAPFSVolumeNoAutomountAtCreateKey;
extern CFStringRef kAPFSVolumeQuotaSizeKey;
extern CFStringRef kAPFSVolumeReserveSizeKey;
extern CFStringRef kAPFSVolumeRoleKey;
#endif
