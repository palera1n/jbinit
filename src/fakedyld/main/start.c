#include <fakedyld/fakedyld.h>

char** environ = NULL;
const struct KernelArgs* gKernArgs;
void* gPreDyldMH;

void start(const struct KernelArgs* kernArgs, void* preDyldMH)
__attribute__((noreturn)) __asm("start");
void start(const struct KernelArgs* kernArgs, void* preDyldMH)  {
    gKernArgs = kernArgs;
    gPreDyldMH = preDyldMH;
    int argc = (int)kernArgs->argc;
    char** argv = (char**)&kernArgs->args[0];
    char** envp = (char**)&kernArgs->args[argc + 1];
    environ = envp;
    char** apple = envp;
    while (*apple != NULL) apple++;
    apple++;
    main(argc, (char**)argv, (char**)envp, (char**)apple);
    panic("main function returned");
}
