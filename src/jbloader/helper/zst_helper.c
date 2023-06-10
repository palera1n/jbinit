// 
//  zst_helper.c
//  src/jbloader/helper/zst_helper.c
//  
//  Created 07/05/2023
//  jbloader (helper)
//

#include <jbloader.h>

#define ZSTD_BIN "/cores/binpack/usr/bin/zstd"
#define TAR_TMP "/var/mobile/Library/palera1n/temp/bootstrap.tar"

int pre_checks() {
    if (access(ZSTD_BIN, F_OK) != 0) {
        fprintf(stderr, "%s %s %s%d%s\n", "could not access zstd:", strerror(errno), "(", errno, ")");
        return errno;
    }

    FILE *bootstrap = fopen(TAR_TMP, "rb");
    if (bootstrap != NULL) {
        fprintf(stdout, "%s\n", "Found existing bootstrap.tar, removing before decompressing.");
        fclose(bootstrap);
        if (remove(TAR_TMP) != 0) {
            fprintf(stderr, "%s\n", "Failed to remove existing bootstrap.tar.");
            return -1;
        }
    } else {
        fclose(bootstrap);
    }

    return 0;
}

int decompress(char *tar_path) {
    int ret;
    pid_t pid;
    int status;

    ret = pre_checks();
    if (ret != 0) {
        fprintf(stderr, "%s\n", "Pre checks for zstd have failed.");
        return -1;
    }
    
    char* args[] = {"zstd", "-d", tar_path, "-o", TAR_TMP, NULL};
    
    ret = posix_spawnp(&pid, ZSTD_BIN, NULL, NULL, args, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s %d", "zstd failed with:", ret);
        return ret;
    }

    waitpid(pid, &status, 0);
    return 0;
}
