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

void reload_launchd_env(void) {
	xpc_object_t launchd_dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_object_t launchd_reply;
    xpc_dictionary_set_uint64(launchd_dict, "cmd", LAUNCHD_CMD_RELOAD_JB_ENV);
    int ret = jailbreak_send_launchd_message(launchd_dict, &launchd_reply);
    xpc_release(launchd_dict);
}

void NSLog(CFStringRef, ...);
void palera1nd_handler(xpc_object_t peer, xpc_object_t event, struct paleinfo* pinfo);
int palera1nd_main(int argc, char* argv[]) {
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
				free(desc);
		    	if (event == XPC_TYPE_ERROR) {
					return;
				}

				palera1nd_handler(peer, event, (struct paleinfo*)&pinfo);
			});

			xpc_connection_resume(peer);
		} else if (xpc_get_type(peer) == XPC_TYPE_ERROR) {
			if (peer == XPC_ERROR_CONNECTION_INVALID) {
			} else {
			}
			exit(1);
		}
	});

    xpc_connection_activate(connection);
	dispatch_main();
    return -1;
}
