#include <jbloader.h>
#include <math.h>

int print_info_main(int argc, char* argv[]) {
    printf("kerninfo:\n");
    printf("\tkbase: 0x%llx\n\tkslide: 0x%llx\n\tinfo size: 0x%llx\n", info.base, info.slide, info.size);
    printf("\tflags: ");
    for (uint8_t bit = 0; bit < 32; bit++) {
        if (checkrain_option_enabled(info.flags, (1 << bit))) {
            char printbuf[0x30];
            const char* opt_str = str_checkrain_flags(1 << bit);
            if (opt_str == NULL) {
                snprintf(printbuf, 0x30, "checkrain_option_unknown_%.0lf", log2((1 << bit)));
            }
            printf("%s ", opt_str == NULL ? printbuf : opt_str);
        }
    }
    puts("");
    printf("paleinfo:\n");
    printf("\tmagic: 0x%x\n", pinfo.magic);
    printf("\tversion: %d\n", pinfo.version);
    printf("\trootdev: %s\n", checkrain_option_enabled(pinfo.flags, palerain_option_rootful) ? pinfo.rootdev : "");
    printf("\tflags: ");
    for (uint8_t bit = 0; bit < 32; bit++) {
        if (checkrain_option_enabled(pinfo.flags, (1 << bit))) {
            char printbuf[0x30];
            const char* opt_str = str_palerain_flags(1 << bit);
            if (opt_str == NULL) {
                snprintf(printbuf, 0x30, "palerain_option_unknown_%.0lf", log2((1 << bit)));
            }
            printf("%s ", opt_str == NULL ? printbuf : opt_str);
        }
    }
    puts("");
    return 0;
}
