#ifndef _STRACE_H_
#define _STRACE_H_

#include <common.h>

void strace_call(uintptr_t pc, char *name);
void print_stracebuf();

#endif