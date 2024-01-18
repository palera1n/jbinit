#include <libjailbreak/libjailbreak.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <os/alloc_once_private.h>
#include <xpc/private.h>

int jailbreak_send_launchd_message(xpc_object_t xdict, xpc_object_t *xreply) {
    int ret = 0;
    xpc_dictionary_set_bool(xdict, "jailbreak", true);
	xpc_object_t bootstrap_pipe = ((struct xpc_global_data *)_os_alloc_once_table[OS_ALLOC_ONCE_KEY_LIBXPC].ptr)->xpc_bootstrap_pipe;
	if (__builtin_available(macOS 12.0, iOS 15.0, tvOS 15.0, watchOS 8.0, *)) {
		ret = _xpc_pipe_interface_routine(bootstrap_pipe, 0, xdict, xreply, 0);
	} else {
		ret = xpc_pipe_routine(bootstrap_pipe, xdict, xreply);
	}
    //ret = xpc_pipe_routine(bootstrap_pipe, xdict, xreply);
	if (ret == 0 && (ret = xpc_dictionary_get_int64(*xreply, "error")) == 0)
		return 0;

    return ret;
}
