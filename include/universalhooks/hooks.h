#ifndef UNIVERSALHOOKS_HOOKS_H
#define UNIVERSALHOOKS_HOOKS_H

#include <stdint.h>
#include <stdbool.h>

extern uint64_t pflags;
extern bool rootful;

void lsdRootlessInit(void);
void securitydInit(void);
void watchdogdInit(void);
void springboardInit(void);
void cfprefsdInit(void);
void pineboardInit(void);
void lsdUniversalInit(void);
void headboardInit(void);
void trollstorehelperInit(char* executablePath);

#endif
