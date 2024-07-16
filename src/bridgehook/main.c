#include <stdio.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <string.h>
#include <mach/message.h>
#include <mach/vm_region.h>
#include <mach/vm_map.h>
#include <mach/mach_vm.h>
#include <mach/mach.h>
#include <libkern/OSCacheControl.h>

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

BH_EXPORT
void MSHookClassPair(Class target, Class hook, Class base) {
    uint32_t method_count = 0;
    Method *methods = class_copyMethodList(hook, &method_count);
    if (!methods) return;
    for (uint32_t i = 0; i < method_count; i++) {
        SEL selector = method_getName(methods[i]);
        const char* method_encoding = method_getTypeEncoding(methods[i]);
        Method origImp = class_getInstanceMethod(base, selector);
        Method hookedImp = class_getInstanceMethod(target, selector);
        
        if (origImp && hookedImp) {
            class_addMethod(base, selector, method_getImplementation(methods[i]), method_encoding);
            method_exchangeImplementations(hookedImp, origImp);
        } else {
            class_addMethod(target, selector, method_getImplementation(methods[i]), method_encoding);
        }
    }
    
    free(methods);
}

BH_EXPORT
void* MSHookIvar(id self, const char* name) {
    Ivar ivar = class_getInstanceVariable(object_getClass(self), name);
    if (ivar) {
        void* pointer = ivar == NULL ? NULL : (uint8_t*)self + ivar_getOffset(ivar);
        return pointer;
    }
    return NULL;
}

BH_EXPORT
void MSHookMemory(void *target, const void *data, size_t size) {
    kern_return_t ret = KERN_SUCCESS;
    vm_address_t vmTarget = (vm_address_t)target;
    vm_size_t vmSize = size;
    mach_msg_type_number_t infoCount = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
    natural_t depth = 99999;
    struct vm_region_submap_short_info_64 vmInfo;
    // obtain permission information
    ret = vm_region_recurse_64(mach_task_self(), &vmTarget, &vmSize, &depth, (vm_region_recurse_info_t)&vmInfo, &infoCount);
    
    if (ret) return;
    
    vmTarget = (vm_address_t)target;
    bool rdonly = !(vmInfo.protection & VM_PROT_WRITE);
    
    // make writable
    if (rdonly) {
        int flags = VM_PROT_READ | VM_PROT_WRITE;
        if ((vmInfo.max_protection & VM_PROT_WRITE) == 0) flags |= VM_PROT_COPY;
        
        ret = mach_vm_protect(mach_task_self(), vmTarget, size, 0, flags);
        if (ret) return;
    }
    
    memcpy(target, data, size);
    
    // revert permission changes
    if (rdonly) {
        ret = mach_vm_protect(mach_task_self(), vmTarget, size, 0, vmInfo.protection);
        if (ret) return;
    }
    
    sys_icache_invalidate(target, size);
    return;
}
