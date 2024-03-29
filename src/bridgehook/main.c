#include <stdio.h>
#include <objc/objc.h>
#include <objc/runtime.h>

int DobbyHook(void *address, void *fake_func, void **out_origin_func);

__attribute__ ((visibility ("default")))
void MSHookFunction(void *address, void *fake_func, void **out_origin_func) {
	DobbyHook(address, fake_func, out_origin_func);
}

__attribute__ ((visibility ("default")))
void MSHookMessageEx(Class class_, SEL selector_, IMP replacement, IMP* replacee) {
	Method method = class_getInstanceMethod(class_, selector_);
	if (!method) {
		method = class_getClassMethod(class_, selector_);
		if (!method) return;
    	}
	IMP orig = method_setImplementation(method, replacement);
	if (orig) *replacee = orig;
}
