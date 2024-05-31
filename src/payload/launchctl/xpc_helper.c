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
#include <Availability.h>

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syslimits.h>

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysdir.h>
#include <unistd.h>

#include <mach/mach.h>

#include <xpc/xpc.h>
#include "xpc_private.h"

#include "os_alloc_once.h"

#include "launchctl.h"

int
launchctl_send_xpc_to_launchd(uint64_t routine, xpc_object_t msg, xpc_object_t *reply)
{
	xpc_object_t bootstrap_pipe = ((struct xpc_global_data *)_os_alloc_once_table[OS_ALLOC_ONCE_KEY_LIBXPC].ptr)->xpc_bootstrap_pipe;

	// Routines that act on a specific service are in the subsystem 2
	// but that require a domain are in the subsystem 3 these are also
	// divided into the routine numbers 0x2XX and 0x3XX, so a quick and
	// dirty bit shift will let us get the correct subsystem.
	xpc_dictionary_set_uint64(msg, "subsystem", routine >> 8);
	xpc_dictionary_set_uint64(msg, "routine", routine);
	int ret = 0;

	if (__builtin_available(macOS 12.0, iOS 15.0, tvOS 15.0, watchOS 8.0, bridgeOS 6.0, *)) {
		ret = _xpc_pipe_interface_routine(bootstrap_pipe, 0, msg, reply, 0);
	} else {
		ret = xpc_pipe_routine(bootstrap_pipe, msg, reply);
	}
	if (ret == 0 && (ret = xpc_dictionary_get_int64(*reply, "error")) == 0)
		return 0;

	return ret;
}

void
launchctl_xpc_object_print(xpc_object_t in, const char *name, int level)
{
	for (int i = 0; i < level; i++)
		putchar('\t');

	if (name != NULL)
		printf("\"%s\" = ", name);

	xpc_type_t t = xpc_get_type(in);
	if (t == XPC_TYPE_STRING)
		printf("\"%s\";\n", xpc_string_get_string_ptr(in));
	else if (t == XPC_TYPE_INT64)
		printf("%lld;\n", xpc_int64_get_value(in));
	else if (t == XPC_TYPE_DOUBLE)
		printf("%f;\n", xpc_double_get_value(in));
	else if (t == XPC_TYPE_BOOL) {
		if (in == XPC_BOOL_TRUE)
			printf("true;\n");
		else if (in == XPC_BOOL_FALSE)
			printf("false;\n");
	} else if (t == XPC_TYPE_MACH_SEND)
		printf("mach-port-object;\n");
	else if (t == XPC_TYPE_FD)
		printf("file-descriptor-object;\n");
	else if (t == XPC_TYPE_ARRAY) {
		printf("(\n");
		int c = xpc_array_get_count(in);
		for (int i = 0; i < c; i++) {
			launchctl_xpc_object_print(xpc_array_get_value(in, i), NULL, level + 1);
		}
		for (int i = 0; i < level; i++)
			putchar('\t');
		printf(");\n");
	} else if (t == XPC_TYPE_DICTIONARY) {
		printf("{\n");
		int __block blevel = level + 1;
		(void)xpc_dictionary_apply(in, ^ bool (const char *key, xpc_object_t value) {
				launchctl_xpc_object_print(value, key, blevel);
				return true;
		});
		for (int i = 0; i < level; i++)
			putchar('\t');
		printf("};\n");
	}
}

void
launchctl_setup_xpc_dict(xpc_object_t dict)
{
	if (__builtin_available(macOS 12.0, iOS 15.0, tvOS 15.0, watchOS 8.0, bridgeOS 6.0, *)) {
		xpc_dictionary_set_uint64(dict, "type", 7);
	} else {
		xpc_dictionary_set_uint64(dict, "type", 1);
	}
	xpc_dictionary_set_uint64(dict, "handle", 0);
	return;
}

bool
launchctl_test_xpc_send(uint64_t type, uint64_t handle, const char *name)
{
	xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
	xpc_object_t reply = NULL;
	xpc_dictionary_set_uint64(dict, "type", type);
	xpc_dictionary_set_uint64(dict, "handle", handle);
	xpc_dictionary_set_string(dict, "name", name);
	int err = launchctl_send_xpc_to_launchd(XPC_ROUTINE_UNKNOWN, dict, &reply);
	return err == 0;
}

int
launchctl_setup_xpc_dict_for_service_name(char *servicetarget, xpc_object_t dict, const char **name)
{
	long handle = 0;

	if (name != NULL) {
		*name = NULL;
	}

	const char *split[3] = {NULL, NULL, NULL};
	for (int i = 0; i < 3; i++) {
		char *var = strsep(&servicetarget, "/");
		if (var == NULL)
			break;
		split[i] = var;
	}
	if (split[0] == NULL || split[0][0] == '\0')
		return EBADNAME;

	if (strcmp(split[0], "system") == 0) {
		xpc_dictionary_set_uint64(dict, "type", 1);
		xpc_dictionary_set_uint64(dict, "handle", 0);
		if (split[1] != NULL && split[1][0] != '\0') {
			xpc_dictionary_set_string(dict, "name", split[1]);
			if (name != NULL) {
				*name = split[1];
			}
			if (__builtin_available(macOS 13.0, iOS 16.0, tvOS 16.0, watchOS 9.0, bridgeOS 7.0, *)) {
				if (xpc_user_sessions_enabled() && launchctl_test_xpc_send(1, handle, split[1]) == false) {
					uint64_t fguid = xpc_user_sessions_get_foreground_uid(0);
					if (launchctl_test_xpc_send(2, fguid, split[1])) {
						fprintf(stderr, "Warning: Please switch to user/foreground/%s service identifier\n", split[1]);
						xpc_dictionary_set_uint64(dict, "type", 2);
						xpc_dictionary_set_uint64(dict, "handle", xpc_user_sessions_get_foreground_uid(0));
					}
				}
			}
		}
		return 0;
	} else if (strcmp(split[0], "user") == 0) {
		xpc_dictionary_set_uint64(dict, "type", 2);
		if (split[1] != NULL && strcmp(split[1], "foreground") == 0) {
			if (__builtin_available(macOS 13.0, iOS 16.0, tvOS 16.0, watchOS 9.0, bridgeOS 7.0, *)) {
				if (xpc_user_sessions_enabled() == 0) {
					fprintf(stderr, "user/foreground/ specifier is not supported on this platform\n");
					return ENOTSUP;
				}
				handle = xpc_user_sessions_get_foreground_uid(0);
			}
		}
	} else if (strcmp(split[0], "session") == 0) {
		xpc_dictionary_set_uint64(dict, "type", 4);
	} else if (strcmp(split[0], "pid") == 0) {
		xpc_dictionary_set_uint64(dict, "type", 5);
	} else {
		xpc_dictionary_set_uint64(dict, "type", 9);
	}
	if (split[1] != NULL) {
		if (handle == 0) {
			handle = strtol(split[1], NULL, 10);
			if (handle == -1)
				return EUSAGE;
		}
		xpc_dictionary_set_uint64(dict, "handle", handle);
		if (split[2] != NULL && split[2][0] != '\0') {
			xpc_dictionary_set_string(dict, "name", split[2]);
			if (name != NULL) {
				*name = split[2];
			}
		}
		return 0;
	}
	return EBADNAME;
}

xpc_object_t
launchctl_parse_load_unload(unsigned int domain, int count, char **list)
{
	xpc_object_t ret;
	ret = xpc_array_create(NULL, 0);
	char pathbuf[PATH_MAX*2];
	memset(pathbuf, 0, PATH_MAX*2);

	if (domain != 0) {
		sysdir_search_path_enumeration_state state;
		state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_LIBRARY, SYSDIR_DOMAIN_MASK_LOCAL | SYSDIR_DOMAIN_MASK_SYSTEM);
		while ((state = sysdir_get_next_search_path_enumeration(state, pathbuf)) != 0) {
			strcat(pathbuf, "/LaunchDaemons");
			xpc_array_set_string(ret, XPC_ARRAY_APPEND, pathbuf);
		}
	}

	for (int i = 0; i < count; i++) {
		char *finalpath;
		if (list[i][0] == '/')
			finalpath = strdup(list[i]);
		else {
			getcwd(pathbuf, sizeof(pathbuf));
			asprintf(&finalpath, "%s/%s", pathbuf, list[i]);
		}
		xpc_array_set_string(ret, XPC_ARRAY_APPEND, finalpath);
		free(finalpath);
	}

	return ret;
}

vm_address_t
launchctl_create_shmem(xpc_object_t dict, vm_size_t sz)
{
	vm_address_t addr = 0;
	xpc_object_t shmem;

	vm_allocate(mach_task_self(), &addr, sz, 0xf0000003);
	shmem = xpc_shmem_create((void*)addr, sz);
	xpc_dictionary_set_value(dict, "shmem", shmem);

	return addr;
}

void
launchctl_print_shmem(xpc_object_t dict, vm_address_t addr, vm_size_t sz, FILE *outfd)
{
	uint64_t written;

	written = xpc_dictionary_get_uint64(dict, "bytes-written");
	if (written <= sz) {
		if (written == 0) {
			fwrite("<eof>", 5, 1, outfd);
		} else {
			fwrite((void*)addr, 1, written, outfd);
		}
		fflush(outfd);
		return;
	}
}

xpc_object_t
launchctl_xpc_from_plist(const char *path)
{
	xpc_object_t plist = NULL;
	struct stat sb;
	void *f;
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1)
		return NULL;

	if (fstat(fd, &sb) == -1)
		goto cleanup;

	if ((f = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		goto cleanup;

	plist = xpc_create_from_plist(f, sb.st_size);

	munmap(f, sb.st_size);
cleanup:
	close(fd);
	return plist;
}

void
launchctl_print_domain_str(FILE *s, xpc_object_t msg)
{
	uint64_t type, handle;

	type = xpc_dictionary_get_uint64(msg, "type");
	handle = xpc_dictionary_get_uint64(msg, "handle");

	switch (type) {
		case 1:
			fprintf(s, "system");
			break;
		case 2:
			fprintf(s, "uid: %"PRIu64, handle);
			break;
		case 3:
			fprintf(s, "login: %"PRIu64, handle);
			break;
		case 4:
			fprintf(s, "asid: %"PRIu64, handle);
			break;
		case 5:
			fprintf(s, "pid: %"PRIu64, handle);
			break;
		case 7:
			fprintf(s, "ports");
			break;
		case 8:
			fprintf(s, "user gui: %"PRIu64, handle);
			break;
	}
}
