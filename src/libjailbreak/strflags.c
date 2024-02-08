#include <libjailbreak/libjailbreak.h>
#include <paleinfo.h>

const char* jailbreak_str_pinfo_flag(uint64_t flag) {
    switch (flag) {
        case palerain_option_rootful:
            return "palerain_option_rootful";
        case palerain_option_rootless:
            return "palerain_option_rootless";
        case palerain_option_setup_rootful:
            return "palerain_option_setup_rootful";
        case palerain_option_checkrain_is_clone:
            return "palerain_option_checkrain_is_clone";
        case palerain_option_rootless_livefs:
            return "palerain_option_rootless_livefs";
        case palerain_option_ssv:
            return "palerain_option_ssv";
        case palerain_option_clean_fakefs:
            return "palerain_option_clean_fakefs";
        case palerain_option_tui:
            return "palerain_option_tui";
        case palerain_option_dfuhelper_only:
            return "palerain_option_dfuhelper_only";
        case palerain_option_pongo_exit:
            return "palerain_option_pongo_exit";
        case palerain_option_demote:
            return "palerain_option_demote";
        case palerain_option_pongo_full:
            return "palerain_option_pongo_full";
        case palerain_option_palerain_version:
            return "palerain_option_palerain_version";
        case palerain_option_exit_recovery:
            return "palerain_option_exit_recovery";
        case palerain_option_reboot_device:
            return "palerain_option_reboot_device";
        case palerain_option_enter_recovery:
            return "palerain_option_enter_recovery";
        case palerain_option_device_info:
            return "palerain_option_device_info";
        case palerain_option_no_colors:
            return "palerain_option_no_colors";
        case palerain_option_bind_mount:
            return "palerain_option_bind_mount";
        case palerain_option_overlay:
            return "palerain_option_overlay";
        case palerain_option_force_revert:
            return "palerain_option_force_revert";
        case palerain_option_safemode:
            return "palerain_option_safemode";
        case palerain_option_verbose_boot:
            return "palerain_option_verbose_boot";
        case palerain_option_jbinit_log_to_file:
            return "palerain_option_jbinit_log_to_file";
        case palerain_option_setup_rootful_forced:
            return "palerain_option_setup_rootful_forced";
        case palerain_option_failure:
            return "palerain_option_failure";
        case palerain_option_flower_chain:
            return "palerain_option_flower_chain";
        case palerain_option_test1:
            return "palerain_option_test1";
        case palerain_option_test2:
            return "palerain_option_test2";
        default:
            return "palera1n_option_undefined";
    }
}
