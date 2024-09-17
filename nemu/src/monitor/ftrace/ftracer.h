#ifndef _FTRACER_H_
#define _FTRACER_H_

#include <cpu/cpu.h>
#include "elf_parser.h"

void ftrace_call(vaddr_t pc, vaddr_t dnpc);
void ftrace_ret(vaddr_t pc, vaddr_t dnpc);
void print_ftracebuf();

#endif