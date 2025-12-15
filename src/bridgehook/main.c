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
int DobbyCodePatch(void *address, uint8_t *buffer, uint32_t buffer_size);
void *DobbySymbolResolver(const char *image_name, const char *symbol_name_pattern);

BH_EXPORT
void MSHookFunction(void *address, void *fake_func, void **out_origin_func) {
	DobbyHook(address, fake_func, out_origin_func);
}

BH_EXPORT
void MSHookMessageEx(Class class_, SEL selector_, IMP replacement, IMP* replacee) {
    Method method = class_getInstanceMethod(class_, selector_);

    if (!method) {
        method = class_getClassMethod(class_, selector_);

        if (!method)
            return;
    }

    IMP orig = class_replaceMethod(class_, selector_, replacement, method_getTypeEncoding(method));

    if (!replacee)
        return;

    if (orig) {
        *replacee = orig;
        return;
    }

    Class superclass = class_getSuperclass(class_);

    if (!superclass)
        return;

    IMP superclass_method = class_getMethodImplementation(superclass, selector_);

    if (superclass_method)
        *replacee = superclass_method;
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
void MSCloseImage(__unused const char* file) {
    return;
}

BH_EXPORT
void *MSFindSymbol(MSImageRef image, const char *name) {
    if (image) {
        uint32_t file_index = 0;
        for (uint32_t i = 0; i < _dyld_image_count(); i++) {
            if (image == _dyld_get_image_header(i)) {
                file_index = i;
                break;
            }
        }
        if (file_index)
            return DobbySymbolResolver(_dyld_get_image_name(file_index), name);
        else
            return NULL;
    } else {
        for (uint32_t i = 0; i < _dyld_image_count(); i++) {
            void* sym = DobbySymbolResolver(_dyld_get_image_name(i), name);
            if (sym) return sym;
        }
        return NULL;
    }
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
        
        if (hookedImp) {
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
    DobbyCodePatch(target, (uint8_t*)data, (uint32_t)size);
}

