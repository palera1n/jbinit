#include <jbinit.h>
#include <stdint.h>

#include "plooshfinder.h"
#include "ios15.h"
#include "ios16.h"
#include "old.h"

void *dyld_buf;
size_t dyld_len;
int platform = 0;

void patch_platform_check() {
    // this patch tricks dyld into thinking everything is for the current platform
    struct section_64 *text_section = macho_find_section(dyld_buf, "__TEXT", "__text");
    if (!text_section) return;

    void *section_addr = dyld_buf + text_section->offset;
    uint64_t section_len = text_section->size;

    patch_platform_check15(section_addr, section_len, platform);
    patch_platform_check16(section_addr, section_len, platform);
    patch_platform_check_old(section_addr, section_len, platform);
}

void patch_dyld() {
    puts("Plooshi(TM) libDyld64Patcher starting up...");
    puts("patching dyld...");
    dyld_buf = read_file("/usr/lib/dyld", &dyld_len);
    
    uint32_t magic = macho_get_magic(dyld_buf);
    if (!magic) {
        puts("detected corrupted dyld");
        spin();
    }
    void *orig_dyld_buf = dyld_buf;
    if (magic == 0xbebafeca) {
        dyld_buf = macho_find_arch(dyld_buf, CPU_TYPE_ARM64);
        if (!dyld_buf) {
            puts("detected unsupported or invalid dyld architecture");
            spin();
        }
    }
    platform = macho_get_platform(dyld_buf);
    if (platform == 0) {
        puts("detected unsupported or invalid platform");
        spin();
    }
    patch_platform_check();
    write_file("/cores/dyld", dyld_buf, dyld_len);
    puts("done patching dyld");
}
