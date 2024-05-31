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
#include <stdint.h>
#include <stdio.h>

#include <mach/mach.h>

#include <xpc/xpc.h>

#ifndef _LAUNCHCTL_H_
#define _LAUNCHCTL_H_

typedef int cmd_main(xpc_object_t *, int, char **, char **, char **);

// launchctl.c
cmd_main help_cmd;
cmd_main config_cmd;
cmd_main submit_cmd;
cmd_main todo_cmd; // Placeholder

// version.c
cmd_main version_cmd;

// list.c
cmd_main list_cmd;

// examine.c
cmd_main examine_cmd;

// start_stop.c
cmd_main stop_cmd;
cmd_main start_cmd;

// print.c
cmd_main print_cmd;
cmd_main print_cache_cmd;
cmd_main print_disabled_cmd;
cmd_main dumpstate_cmd;

// env.c
cmd_main setenv_cmd;
cmd_main getenv_cmd;

// load.c
cmd_main load_cmd;

// enable.c
cmd_main enable_cmd;

// reboot.c
cmd_main reboot_cmd;

// bootstrap.c
cmd_main bootstrap_cmd;
cmd_main bootout_cmd;

// error.c
cmd_main error_cmd;

// remove.c
cmd_main remove_cmd;

// kickstart.c
cmd_main kickstart_cmd;

// kill.c
cmd_main kill_cmd;

// blame.c
cmd_main blame_cmd;

// manager.c
cmd_main managerpid_cmd;
cmd_main manageruid_cmd;
cmd_main managername_cmd;

// attach.c
cmd_main attach_cmd;

// plist.c
cmd_main plist_cmd;

// runstats.c
cmd_main runstats_cmd;

// userswitch.c
cmd_main userswitch_cmd;

// limit.c
cmd_main limit_cmd;

void launchctl_xpc_object_print(xpc_object_t, const char *name, int level);
int launchctl_send_xpc_to_launchd(uint64_t routine, xpc_object_t msg, xpc_object_t *reply);
void launchctl_setup_xpc_dict(xpc_object_t dict);
int launchctl_setup_xpc_dict_for_service_name(char *servicetarget, xpc_object_t dict, const char **name);
void launchctl_print_domain_str(FILE *s, xpc_object_t msg);
xpc_object_t launchctl_parse_load_unload(unsigned int domain, int count, char **list);
vm_address_t launchctl_create_shmem(xpc_object_t, vm_size_t);
void launchctl_print_shmem(xpc_object_t dict, vm_address_t addr, vm_size_t sz, FILE *outfd);
xpc_object_t launchctl_xpc_from_plist(const char *path);
#endif
