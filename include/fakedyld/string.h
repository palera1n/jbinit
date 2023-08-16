#ifndef FAKEDYLD_STRING_H
#define FAKEDYLD_STRING_H

#include <stddef.h>
extern const char* const sys_errlist[];

size_t strlen(const char* str);
char *strstr(const char *string, char *substring);
char *strchr(const char *p, int ch);
int strncmp(const char* s1, const char* s2, size_t n);
int strcmp(const char* s1, const char* s2);
unsigned long strtoul(const char *nptr, char **endptr, register int base);
unsigned long long strtoull(const char *nptr, char **endptr, register int base);

const char* strerror(int err);
#endif
