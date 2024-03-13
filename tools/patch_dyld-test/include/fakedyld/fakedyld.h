#ifndef MOCK_FAKEDYLD_H
#define MOCK_FAKEDYLD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>

typedef struct {
    char* file_p;
    size_t file_len;
} memory_file_handle_t;


void patch_dyld(memory_file_handle_t* dyld_handle, int platform);
void panic(const char* format, ...);

#define LOG(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)

#endif
