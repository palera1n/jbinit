#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <payload/payload.h>
#include <paleinfo.h>
#include <CoreFoundation/CoreFoundation.h>
#include <os/log.h>
#include <libjailbreak/libjailbreak.h>
#include <dlfcn.h>
#include <os/log.h>

void reload_launchd_env(void) {
	xpc_object_t launchd_dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_object_t launchd_reply;
    xpc_dictionary_set_uint64(launchd_dict, "cmd", LAUNCHD_CMD_RELOAD_JB_ENV);
    jailbreak_send_launchd_message(launchd_dict, &launchd_reply);
    xpc_release(launchd_dict);
	xpc_release(launchd_reply);
}

ssize_t write_fdout(int fd, void* buf, size_t len) {
    ssize_t to_write = len, written = 0;
    uint8_t* buf_current = buf;
    do {
        if (to_write > INT_MAX) to_write = INT_MAX;
        ssize_t didWrite = write(fd, buf_current, to_write);
        if (didWrite == -1) return -1;
        written += didWrite;
        len -= didWrite;
        to_write = len;
        buf_current += didWrite;
    } while (len > 0);
    return written;
}

void palera1nd_handler(xpc_object_t peer, xpc_object_t event, struct paleinfo* pinfo);
int palera1nd_main(int __unused argc, char* __unused argv[]) {
    PALERA1ND_LOG_DEBUG("starting palera1nd");
    struct paleinfo pinfo;
    int ret = get_pinfo(&pinfo);
    if (ret) return -1;

    xpc_connection_t connection = xpc_connection_create_mach_service("in.palera.palera1nd.systemwide", dispatch_get_main_queue(), XPC_CONNECTION_MACH_SERVICE_LISTENER);

    xpc_connection_set_event_handler(connection, ^(xpc_object_t peer) {
		char* desc = xpc_copy_description(peer);
		free(desc);
		if (xpc_get_type(peer) == XPC_TYPE_CONNECTION) {
			xpc_connection_set_event_handler(peer, ^(xpc_object_t event) {
				char* desc = xpc_copy_description(event);
		    	if (xpc_get_type(event) == XPC_TYPE_ERROR) {
                    PALERA1ND_LOG_DEBUG("received error dictionary: %s", desc);
                    free(desc);
					return;
				}
                free(desc);

				palera1nd_handler(peer, event, (struct paleinfo*)&pinfo);
			});

			xpc_connection_resume(peer);
		} else if (xpc_get_type(peer) == XPC_TYPE_ERROR) {
            return;
		}
	});

    xpc_connection_activate(connection);
	dispatch_main();
    return -1;
}
