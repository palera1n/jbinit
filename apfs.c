#include <stdio.h>
#include <mach/mach.h>

extern kern_return_t APFSVolumeDelete(const char *dev);

int main(int argc, const char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s diskN\n", argv[0]);
        return -1;
    }
    kern_return_t ret = APFSVolumeDelete(argv[1]);
    if(ret != KERN_SUCCESS)
    {
        fprintf(stderr, "APFSVolumeDelete: %s\n", mach_error_string(ret));
        return -1;
    }
    return 0;
}
