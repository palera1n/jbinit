#ifndef FAKEDYLD_SPAWN_H
#define FAKEDYLD_SPAWN_H

#include <fakedyld/types.h>
#include <fakedyld/param.h>

int posix_spawn(pid_t *pid, const char *path, const void* adesc, char **argv, char **envp);

#endif
