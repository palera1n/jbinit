#include <jbloader.h>
#include <math.h>

int print_info(int argc, char *argv[]) {
    printf("kerninfo:\n");
    printf("\tkbase: 0x%llx\n\tkslide: 0x%llx\n\tinfo size: 0x%llx\n", info.base, info.slide, info.size);
    printf("\tflags: ");
    print_flag_text(info.flags, "checkrain", str_checkrain_flags);
    puts("");
    printf("paleinfo:\n");
    printf("\tmagic: 0x%x\n", pinfo.magic);
    printf("\tversion: %d\n", pinfo.version);
    printf("\trootdev: %s\n", checkrain_option_enabled(pinfo.flags, palerain_option_rootful) ? pinfo.rootdev : "");
    printf("\tflags: ");
    print_flag_text(pinfo.flags, "palerain", str_palerain_flags);
    puts("");
    return 0;
}
