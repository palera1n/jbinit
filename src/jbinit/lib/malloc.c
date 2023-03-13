#include <jbinit.h>

void* malloc(size_t n) {
    void* addr = mmap(NULL, n + sizeof(size_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (addr == MAP_FAILED) return NULL;
    *((size_t*)addr) = (n + sizeof(size_t));
    return (void*)((char*)addr + sizeof(size_t));
}

void free(void *ptr) {
    if (ptr == NULL) return;
    munmap((void*)((char*)ptr-4), *(size_t*)((char*)ptr-sizeof(size_t)));
    return;
}
