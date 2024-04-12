#include <fakedyld/fakedyld.h>

/* 
 * The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL
 * NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and
 * "OPTIONAL" in this document are to be interpreted as described in
 * RFC 2119.
*/

/* 
 * paleinfo sanity checks
 * unlike in old jbinit, the check here should concern the
 * internal consistency of the paleinfo only.
 * That is, do not consider boot args.
 */
void pinfo_check(struct paleinfo* pinfo_p) {
    /* flags that MUST NOT be set at the same time */
    struct {
        uint64_t disallowed_combination;
        char* message;
    } disallowed_combinations[] = {
        {palerain_option_rootless | palerain_option_rootful, "cannot rootless and rootful at the same time"},
        {palerain_option_clean_fakefs | palerain_option_force_revert, "cannot force revert and clean fakefs at the same time"},
        {palerain_option_rootless_livefs | palerain_option_rootful, "cannot use rootless livefs option on rootful"},
        {palerain_option_clean_fakefs | palerain_option_setup_rootful, "canoot setup fakefs whlist cleaning fakefs"},
        {0, NULL}
    };
    for (uint8_t i = 0; disallowed_combinations[i].disallowed_combination != 0; i++) {
        if ((pinfo_p->flags & disallowed_combinations[i].disallowed_combination) == disallowed_combinations[i].disallowed_combination) {
            panic("%s", disallowed_combinations[i].message);
        }
    }

    /* flags that MUST NOT be set when ramdisk is booted */
    if ((pinfo_p->flags & (
        palerain_option_dfuhelper_only |
        palerain_option_pongo_exit |
        palerain_option_demote |
        palerain_option_palerain_version |
        palerain_option_exit_recovery |
        palerain_option_reboot_device |
        palerain_option_enter_recovery |
        palerain_option_device_info
    )) != 0) {
        panic("ramdisk should never be booted with the options specified");
    }

    if ((pinfo_p->flags & palerain_option_rootful) && pinfo_p->rootdev[0] == '\0') {
        panic("rootful requires rootdev to be set");
    }
    return;
}
