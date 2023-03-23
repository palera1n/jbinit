#include <jbinit.h>

int p1_log(const char* __restrict format, ...) {
    printf("jbinit: ");
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}
