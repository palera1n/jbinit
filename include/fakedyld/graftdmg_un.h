#ifndef _GRAFTDMG_UN_
#define _GRAFTDMG_UN_

#include <stdint.h>

#define GRAFTDMG_SECURE_BOOT_CRYPTEX_ARGS_VERSION 1
#define MAX_GRAFT_ARGS_SIZE 512

/* Flag values for secure_boot_cryptex_args.sbc_flags */
#define SBC_PRESERVE_MOUNT              0x0001  /* Preserve underlying mount until shutdown */
#define SBC_ALTERNATE_SHARED_REGION     0x0002  /* Binaries within should use alternate shared region */
#define SBC_SYSTEM_CONTENT              0x0004  /* Cryptex contains system content */
#define SBC_PANIC_ON_AUTHFAIL           0x0008  /* On failure to authenticate, panic */
#define SBC_STRICT_AUTH                 0x0010  /* Strict authentication mode */
#define SBC_PRESERVE_GRAFT              0x0020  /* Preserve graft itself until unmount */

typedef struct secure_boot_cryptex_args {
	uint32_t sbc_version; /* 1 */
	uint32_t sbc_4cc; /* ??? */
	int sbc_authentic_manifest_fd; /* apticket im4m ??? */
	int sbc_user_manifest_fd; /* dmg trustcache im4p */
	int sbc_payload_fd; /* dmg root hash */
	uint64_t sbc_flags;
} __attribute__((aligned(4), packed))  secure_boot_cryptex_args_t;

typedef union graft_args {
	uint8_t max_size[MAX_GRAFT_ARGS_SIZE];
	secure_boot_cryptex_args_t sbc_args;
} graftdmg_args_un;
#endif /* _GRAFTDMG_UN_ */
