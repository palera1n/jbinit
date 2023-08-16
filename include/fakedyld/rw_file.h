#ifndef FAKEDYLD_RWFILE_H
#define FAKEDYLD_RWFILE_H

#include <stddef.h>

typedef struct {
    char* file_p;
    size_t file_len;
} memory_file_handle_t;

void read_file(char* path, memory_file_handle_t* handle);
/* WARNING: using this function will DESTROY the handle */
void write_file(char* path, memory_file_handle_t* handle);

#endif
