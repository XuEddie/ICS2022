#ifndef _ETRACE_H_
#define _ETRACE_H_

#include <cpu/cpu.h>
#include "../ftrace/elf_parser.h"

void etrace_call(vaddr_t pc, vaddr_t dnpc);
void etrace_ret(vaddr_t pc, vaddr_t dnpc);
void print_etracebuf();

#endif