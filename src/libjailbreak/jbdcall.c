#include <libjailbreak/libjailbreak.h>
#include <xpc/xpc.h>
#include <xpc/connection.h>

xpc_object_t jailbreak_send_jailbreakd_message_with_reply_sync(xpc_object_t xdict) {
    xpc_connection_t connection = xpc_connection_create_mach_service("in.palera.palera1nd", NULL, 0);
    if (xpc_get_type(connection) == XPC_TYPE_ERROR) {
        return connection;
    }
    xpc_connection_set_event_handler(connection, ^(xpc_object_t _) {});
    xpc_connection_activate(connection);
    xpc_object_t xreply = xpc_connection_send_message_with_reply_sync(connection, xdict);
    xpc_connection_cancel(connection);
    xpc_release(connection);
    return xreply;
}

xpc_object_t jailbreak_send_jailbreakd_command_with_reply_sync(uint64_t cmd) {
    xpc_object_t xdict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(xdict, "cmd", cmd);
    xpc_object_t xreply = jailbreak_send_jailbreakd_message_with_reply_sync(xdict);
    xpc_release(xdict);
    return xreply;
}
