#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "plooshfinder.h"

extern void *dyld_buf;
extern size_t dyld_len;
extern int platform;



int main(int argc, char **argv) {
    FILE *fp = NULL;

    if (argc < 3) {
        printf("Usage: %s <input dyld> <patched dyld>\n", argv[0]);
        return 0;
    }

    fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Failed to open dyld!\n");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    dyld_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    dyld_buf = (void *)malloc(dyld_len);
    if (!dyld_buf) {
        printf("Out of memory while allocating region for dyld!\n");
        fclose(fp);
        return -1;
    }

    fread(dyld_buf, 1, dyld_len, fp);
    fclose(fp);

    if (get_platform() != 0) {
        printf("Failed to get platform!\n");
        return 1;
    }

    patch_platform_check();

    fp = fopen(argv[2], "wb+");
    if(!fp) {
        printf("Failed to open output file!\n");
        free(dyld_buf);
        return -1;
    }
    
    fwrite(dyld_buf, 1, dyld_len, fp);
    fflush(fp);
    fclose(fp);

    free(dyld_buf);

    return 0;
}
