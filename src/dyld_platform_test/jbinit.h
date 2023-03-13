#ifndef JBINIT_H
#define JBINIT_H

#if __STDC_HOSTED__ != 1
#error "this file is not for jbinit use"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static inline void* read_file(char* __attribute__((unused)) path, size_t* __attribute__((unused)) size) { abort(); return NULL; };
static inline int write_file(char* __attribute__((unused)) path, void* __attribute__((unused)) data, size_t __attribute__((unused)) size) { abort(); return 0; };
static inline void spin() { abort(); return; };
void patch_platform_check();
int get_platform();

#endif
