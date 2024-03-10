#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include "include/fakedyld/fakedyld.h"
#include <unistd.h>

void panic(const char* format, ...) {
    va_list va;
    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
    fprintf(stderr, "\n");
    abort();
}

void *dyld_buf;
size_t dyld_len;

int main(int argc, char **argv) {
    FILE *fp = NULL;
    char *outfile = NULL, *patch_dy = NULL;

    int ch;
    while ((ch = getopt(argc, (char * const *)argv, "o:")) != -1) {
        switch(ch) {
            case 'o':
                outfile = optarg;
                break;
       }
    }

    patch_dy = argv[0];
    argc -= optind;
    argv += optind;


    if (argc < 1) {
        printf("Usage: %s -o <patched dyld> <input dyld>\n", patch_dy);
        return -1;
    }


    for (uint32_t i = 0; argv[i] != NULL; i++) {
        char* infile = argv[i];

        fp = fopen(infile, "rb");
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

        memory_file_handle_t handle;
        handle.file_p = dyld_buf;
        handle.file_len = dyld_len;

        printf("infile: %s\n", infile);
        patch_dyld(&handle, 2);

        if (outfile) {
            fp = fopen(outfile, "wb");
            if(!fp) {
                printf("Failed to open output file!\n");
                free(dyld_buf);
                return -1;
            }

            fwrite(dyld_buf, 1, dyld_len, fp);
            fflush(fp);
            fclose(fp);
            free(dyld_buf);
        }
    }


    return 0;
}
