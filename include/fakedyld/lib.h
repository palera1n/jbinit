#ifndef FAKEDYLD_LIB_H
#define FAKEDYLD_LIB_H
#include <fakedyld/types.h>
#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int isdigit(int c);
int isspace(int c);
int atoi(const char* str);
void bzero (void *s, size_t n);
int isupper(int c);
int isalpha(int c);
void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);
int fs_snapshot_list(int dirfd, struct attrlist *alist, void *attrbuf, size_t bufsize, uint32_t flags);
int fs_snapshot_create(int dirfd, const char *name, uint32_t flags);
int fs_snapshot_delete(int dirfd, const char *name, uint32_t flags);
int fs_snapshot_rename(int dirfd, const char *old, const char *new, uint32_t flags);
int fs_snapshot_revert(int dirfd, const char *name, uint32_t flags);
int fs_snapshot_root(int dirfd, const char *name, uint32_t flags);
int fs_snapshot_mount(int dirfd, const char *dir, const char *snapshot, uint32_t flags);

#endif
