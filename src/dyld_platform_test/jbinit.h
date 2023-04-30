#ifndef JBINIT_H
#define JBINIT_H

#if __STDC_HOSTED__ != 1
#error "this file is not for jbinit use"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define LOG printf

static inline void* read_file(char* __attribute__((unused)) path, size_t* __attribute__((unused)) size) { assert(0); return NULL; };
static inline int write_file(char* __attribute__((unused)) path, void* __attribute__((unused)) data, size_t __attribute__((unused)) size) { assert(0); return 0; };
static inline void spin() { assert(0); return; };
void patch_dyld(void);

#endif
