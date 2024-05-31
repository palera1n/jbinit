/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022-2023 Procursus Team <team@procurs.us>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stddef.h>
#include <stdint.h>

#include <xpc/xpc.h>

#ifndef _LAUNCHCTL_XPC_PRIVATE_H_
#define _LAUNCHCTL_XPC_PRIVATE_H_

enum {
	XPC_ROUTINE_KICKSTART_SERVICE = 702,
	XPC_ROUTINE_ATTACH_SERVICE = 703,
	XPC_ROUTINE_BLAME_SERVICE = 707,
	XPC_ROUTINE_PRINT_SERVICE = 708,
	XPC_ROUTINE_RUNSTATS = 709,
	XPC_ROUTINE_UNKNOWN = 712,
	XPC_ROUTINE_LOAD = 800,
	XPC_ROUTINE_UNLOAD = 801,
	XPC_ROUTINE_ENABLE = 808,
	XPC_ROUTINE_DISABLE = 809,
	XPC_ROUTINE_SERVICE_KILL = 812,
	XPC_ROUTINE_SERVICE_START = 813,
	XPC_ROUTINE_SERVICE_STOP = 814,
	XPC_ROUTINE_LIST = 815,
	XPC_ROUTINE_REMOVE = 816,
	XPC_ROUTINE_SETENV = 819,
	XPC_ROUTINE_GETENV = 820,
	XPC_ROUTINE_LIMIT = 825,
	XPC_ROUTINE_EXAMINE = 826,
	XPC_ROUTINE_PRINT = 828,
	XPC_ROUTINE_DUMPSTATE = 834,
};

XPC_DECL(xpc_pipe);

XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL1 XPC_NONNULL2 XPC_NONNULL3
int
xpc_pipe_routine(xpc_pipe_t pipe, xpc_object_t message,
	xpc_object_t XPC_GIVES_REFERENCE *reply);

XPC_EXPORT XPC_WARN_RESULT XPC_NONNULL1 XPC_NONNULL3 XPC_NONNULL4
int
_xpc_pipe_interface_routine(xpc_pipe_t pipe, uint64_t routine,
	xpc_object_t message, xpc_object_t XPC_GIVES_REFERENCE *reply,
	uint64_t flags) __API_AVAILABLE(ios(15.0), tvos(15.0), watchos(8.0), bridgeos(6.0));

int launch_active_user_switch(long, long) __API_AVAILABLE(ios(15.0), tvos(15.0), watchos(8.0), bridgeos(6.0));

int64_t xpc_user_sessions_enabled(void) __API_AVAILABLE(ios(16.0), tvos(16.0), watchos(9.0), bridgeos(7.0));
uint64_t xpc_user_sessions_get_foreground_uid(uint64_t) __API_AVAILABLE(ios(16.0), tvos(16.0), watchos(9.0), bridgeos(7.0));

XPC_EXPORT XPC_RETURNS_RETAINED XPC_WARN_RESULT XPC_NONNULL1
xpc_object_t xpc_create_from_plist(const void * data, size_t length);

const char *xpc_strerror(int);

#define XPC_TYPE_MACH_SEND (&_xpc_type_mach_send)
XPC_EXPORT
XPC_TYPE(_xpc_type_mach_send);

typedef void (*xpc_dictionary_applier_f)(const char *key, xpc_object_t val, void *ctx);
void xpc_dictionary_apply_f(xpc_object_t xdict, void *ctx, xpc_dictionary_applier_f applier);

typedef void (*xpc_array_applier_f)(size_t index, xpc_object_t value, void* context);
void xpc_array_apply_f(xpc_object_t xarray, void *context, xpc_array_applier_f applier);

enum {
	ENODOMAIN = 112,
	ENOSERVICE = 113,
	E2BIMPL = 116,
	EUSAGE = 117,
	EBADRESP = 118,
	EDEPRECATED = 126,
	EMANY = 133,
	EBADNAME = 140,
	ENOTDEVELOPMENT = 142,
	EWTF = 153,
};

#endif
