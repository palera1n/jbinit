#include <stdio.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <string.h>

#define BH_EXPORT __attribute__ ((visibility ("default")))

typedef void* MSImageRef;

int DobbyHook(void *address, void *fake_func, void **out_origin_func);

BH_EXPORT
void MSHookFunction(void *address, void *fake_func, void **out_origin_func) {
	DobbyHook(address, fake_func, out_origin_func);
}

BH_EXPORT
void MSHookMessageEx(Class class_, SEL selector_, IMP replacement, IMP* replacee) {
	Method method = class_getInstanceMethod(class_, selector_);
	if (!method) {
		method = class_getClassMethod(class_, selector_);
		if (!method) return;
    	}
	IMP orig = method_setImplementation(method, replacement);
	if (orig) *replacee = orig;
}

BH_EXPORT
MSImageRef MSGetImageByName(const char *file) {
    uint32_t file_index = 0;
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        if(!strcmp(_dyld_get_image_name(i), file)) {
            file_index = i;
            break;
        }
    }
    if (file_index)
        return (void*)_dyld_get_image_header(file_index);
    else return NULL;
}

BH_EXPORT
void MSCloseImage(const char* file) {
    return;
}

BH_EXPORT
void *MSFindSymbol(MSImageRef image, const char *name) {
    void* buf = (char*)image;
    
    struct load_command *after_header = buf + sizeof(struct mach_header_64);
    struct mach_header_64 *header = buf;
    struct symtab_command *symtab_cmd = NULL;
    
    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (after_header->cmd == LC_SYMTAB) {
            symtab_cmd = (struct symtab_command *) after_header;

            break;
        }

        after_header = (struct load_command *) ((char *) after_header + after_header->cmdsize);
    }
    
    if (!symtab_cmd) return NULL;

    struct nlist_64 *symtab = buf + symtab_cmd->symoff;
    char *strtab = buf + symtab_cmd->stroff;
    
    for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
        struct nlist_64 *symbol_nlist = symtab + i;
        char *sym_name = strtab + symbol_nlist->n_un.n_strx;

        if (strcmp(sym_name, name) == 0) {
            return (buf + symbol_nlist->n_value);
        }
    }
    
    return NULL;
}

