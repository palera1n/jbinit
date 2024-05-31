/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 Procursus Team <team@procurs.us>
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
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <xpc/xpc.h>
#include "xpc_private.h"

#include "launchctl.h"

int
bootstrap_cmd(xpc_object_t *msg, int argc, char **argv, char **envp, char **apple)
{
	int ret;
	const char *name = NULL;
	xpc_object_t dict, reply, paths;

	if (argc < 2)
		return EUSAGE;

	dict = xpc_dictionary_create(NULL, NULL, 0);
	*msg = dict;
	if (strcmp(argv[1], "--angel") == 0) {
		xpc_dictionary_set_bool(dict, "angel", true);
		argc--;
		argv++;
	}
	ret = launchctl_setup_xpc_dict_for_service_name(argv[1], dict, &name);
	if (ret != 0)
		return ret;
	if (name != NULL)
		return EBADNAME;

	if (argc > 2) {
		paths = launchctl_parse_load_unload(0, argc - 2, argv + 2);
		xpc_dictionary_set_value(dict, "paths", paths);
		if (__builtin_available(macOS 13.0, iOS 16.0, tvOS 16.0, watchOS 9.0, bridgeOS 7.0, *)) {
			if (xpc_dictionary_get_uint64(dict, "type") == 1 && xpc_user_sessions_enabled() != 0) {
				xpc_array_apply(paths, ^bool (size_t index, xpc_object_t val) {
						xpc_object_t plist = launchctl_xpc_from_plist(xpc_string_get_string_ptr(val));
						if (plist != NULL && xpc_get_type(plist) == XPC_TYPE_DICTIONARY) {
							if (xpc_dictionary_get_value(plist, "LimitLoadToSessionType") != 0) {
								xpc_release(plist);
								return true;
							}
						}
						xpc_dictionary_set_uint64(dict, "type", 2);
						xpc_dictionary_set_uint64(dict, "handle", xpc_user_sessions_get_foreground_uid(0));
						return false;
				});
			}
		}
	}
	ret = launchctl_send_xpc_to_launchd(XPC_ROUTINE_LOAD, dict, &reply);
	if (ret != ENODOMAIN) {
		if (ret == 0) {
			xpc_object_t errors;
			errors = xpc_dictionary_get_value(reply, "errors");
			if (errors == NULL || xpc_get_type(errors) != XPC_TYPE_DICTIONARY)
				return 0;
			(void)xpc_dictionary_apply(errors, ^bool(const char *key, xpc_object_t value) {
					if (xpc_get_type(value) == XPC_TYPE_INT64) {
						int64_t err = xpc_int64_get_value(value);
						if (err == EEXIST || err == EALREADY)
							fprintf(stderr, "%s: service already bootstrapped\n", key);
						else
							fprintf(stderr, "%s: %s\n", key, xpc_strerror(err));
					}
					return true;
			});
			int64_t err;
			if ((err = xpc_dictionary_get_int64(reply, "bootstrap-error")) == 0)
				return 0;
			else {
				fprintf(stderr, "Bootstrap failed: %lld: %s\n", err, strerror(err));
				return err;
			}
		}
		fprintf(stderr, "Bootstrap failed: %d: %s\n", ret, xpc_strerror(ret));
	}

	return ret;
}

int
bootout_cmd(xpc_object_t *msg, int argc, char **argv, char **envp, char **apple)
{
	int ret;
	const char *name = NULL;
	xpc_object_t dict, reply, paths;

	if (argc < 2)
		return EUSAGE;

	dict = xpc_dictionary_create(NULL, NULL, 0);
	*msg = dict;
	ret = launchctl_setup_xpc_dict_for_service_name(argv[1], dict, &name);
	if (ret != 0)
		return ret;

	if (argc > 2 && name == NULL) {
		paths = launchctl_parse_load_unload(0, argc - 2, argv + 2);
		xpc_dictionary_set_value(dict, "paths", paths);
		if (__builtin_available(macOS 13.0, iOS 16.0, tvOS 16.0, watchOS 9.0, bridgeOS 7.0, *)) {
			if (xpc_dictionary_get_uint64(dict, "type") == 1 && xpc_user_sessions_enabled() != 0) {
				xpc_array_apply(paths, ^bool (size_t index, xpc_object_t val) {
						xpc_object_t plist = launchctl_xpc_from_plist(xpc_string_get_string_ptr(val));
						if (plist != NULL && xpc_get_type(plist) == XPC_TYPE_DICTIONARY) {
							if (xpc_dictionary_get_value(plist, "LimitLoadToSessionType") != 0) {
								xpc_release(plist);
								return true;
							}
						}
						xpc_dictionary_set_uint64(dict, "type", 2);
						xpc_dictionary_set_uint64(dict, "handle", xpc_user_sessions_get_foreground_uid(0));
						return false;
				});
			}
		}
	}

	if (__builtin_available(macOS 12.0, iOS 15.0, tvOS 15.0, watchOS 8.0, bridgeOS 6.0, *)) {
		xpc_dictionary_set_bool(dict, "no-einprogress", true);
	}

	ret = launchctl_send_xpc_to_launchd(XPC_ROUTINE_UNLOAD, dict, &reply);
	if (ret != ENODOMAIN) {
		if (ret == 0) {
			xpc_object_t errors;
			errors = xpc_dictionary_get_value(reply, "errors");
			if (errors == NULL || xpc_get_type(errors) != XPC_TYPE_DICTIONARY)
				return 0;
			(void)xpc_dictionary_apply(errors, ^bool(const char *key, xpc_object_t value) {
					if (xpc_get_type(value) == XPC_TYPE_INT64) {
						int64_t err = xpc_int64_get_value(value);
						if (err == EEXIST || err == EALREADY)
							fprintf(stderr, "%s: service already booted out\n", key);
						else
							fprintf(stderr, "%s: %s\n", key, xpc_strerror(err));
					}
					return true;
			});
			int64_t err;
			if ((err = xpc_dictionary_get_int64(reply, "bootout-error")) == 0)
				return 0;
			else {
				fprintf(stderr, "Bootout failed: %lld: %s\n", err, strerror(err));
				return err;
			}
		}
		fprintf(stderr, "Boot-out failed: %d: %s\n", ret, xpc_strerror(ret));
	}

	return ret;
}
