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
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sysdir.h>

#include <xpc/xpc.h>

#include "xpc_private.h"

#include "launchctl.h"

int
load_cmd(xpc_object_t *msg, int argc, char **argv, char **envp, char **apple)
{
	if (argc < 2)
		return EUSAGE;

	xpc_object_t dict, reply;
	bool load = strcmp(argv[0], "load") == 0;
	int ret;
	unsigned int domain = 0;
	bool wflag, force;
	wflag = force = false;

	int ch;
	while ((ch = getopt(argc, argv, "wFS:D:")) != -1) {
		switch (ch) {
			case 'w':
				wflag = true;
				break;
			case 'F':
				force = true;
				break;
			case 'D':
				if (strcasecmp(optarg, "all") == 0) {
					domain |= SYSDIR_DOMAIN_MASK_ALL;
				} else if (strcasecmp(optarg, "user") == 0) {
					domain |= SYSDIR_DOMAIN_MASK_USER;
				} else if (strcasecmp(optarg, "local") == 0) {
					domain |= SYSDIR_DOMAIN_MASK_LOCAL;
				} else if (strcasecmp(optarg, "network") == 0) {
					fprintf(stderr, "Ignoring network domain.\n");
				} else if (strcasecmp(optarg, "system") == 0) {
					domain |= SYSDIR_DOMAIN_MASK_SYSTEM;
				} else {
					return EUSAGE;
				}
				break;
			case 'S':
				fprintf(stderr, "Session types are not supported on iOS. Ignoring session specifier: %s\n", optarg);
				break;
			default:
				return EUSAGE;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		return EUSAGE;

	dict = xpc_dictionary_create(NULL, NULL, 0);
	*msg = dict;
	launchctl_setup_xpc_dict(dict);
	xpc_object_t array = launchctl_parse_load_unload(domain, argc, argv);
	xpc_dictionary_set_value(dict, "paths", array);
	if (load) {
		xpc_dictionary_set_bool(dict, "enable", wflag);
	} else {
		xpc_dictionary_set_bool(dict, "disable", wflag);
		if (__builtin_available(macOS 12.0, iOS 15.0, tvOS 15.0, watchOS 8.0, bridgeOS 6.0, *)) {
			xpc_dictionary_set_bool(dict, "no-einprogress", true);
		}
	}
	xpc_dictionary_set_bool(dict, "legacy-load", true);
	if (force)
		xpc_dictionary_set_bool(dict, "force", true);
	ret = launchctl_send_xpc_to_launchd(load ? XPC_ROUTINE_LOAD : XPC_ROUTINE_UNLOAD, dict, &reply);
	if (ret == 0) {
		xpc_object_t errors = xpc_dictionary_get_value(reply, "errors");
		if (errors != NULL && xpc_get_type(errors) == XPC_TYPE_DICTIONARY) {
			(void)xpc_dictionary_apply(errors, ^bool(const char *key, xpc_object_t value) {
					if (xpc_get_type(value) == XPC_TYPE_INT64) {
						int64_t err = xpc_int64_get_value(value);
						if (err == EEXIST || err == EALREADY)
							fprintf(stderr, "%s: service already loaded\n", key);
						else
							fprintf(stderr, "%s: %s\n", key, xpc_strerror(err));
					}
					return true;
			});

		}
	}

	return ret;
}
