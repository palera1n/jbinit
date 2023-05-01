#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <APFS/APFS.h>
#include <APFS/APFSConstants.h>

kern_return_t DeleteAPFSVolumeWithRole(const char* volpath)
{
    kern_return_t ret;
    int16_t role = 0;

    ret = APFSVolumeRole(volpath, &role, NULL);
    if(ret != KERN_SUCCESS)
    {
        fprintf(stderr, "could not find %s: %d (%s)\n", volpath, ret, mach_error_string(ret));
        return ret;
    }
    printf("found apfs volume role: 0x%04x\n", role);
    
    if(role != APFS_VOL_ROLE_RECOVERY)
    {
        fprintf(stderr, "this operation is not permitted. (this apfs volume role is not recovery role [0x%04x != 0x%04x])\n", role, APFS_VOL_ROLE_RECOVERY);
        return KERN_DENIED;
    }
    
    return APFSVolumeDelete(volpath);
}
