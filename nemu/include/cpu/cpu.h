/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#ifndef __CPU_CPU_H__
#define __CPU_CPU_H__

#include <common.h>
#include "decode.h"

void cpu_exec(uint64_t n);

void set_nemu_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);

// watchpoint
bool check_watchpoint();

// iringbuf
void print_iringbuf();
void update_iringbuf(Decode *s);

// ftracer
void ftrace_call(vaddr_t pc, vaddr_t dnpc);
void ftrace_ret(vaddr_t pc, vaddr_t dnpc);
void print_ftracebuf();

// etracer
void etrace_call(vaddr_t pc, vaddr_t dnpc);
void etrace_ret(vaddr_t pc, vaddr_t dnpc);
void print_etracebuf();

#define NEMUTRAP(thispc, code) set_nemu_state(NEMU_END, thispc, code)
#define INV(thispc) invalid_inst(thispc)

// csr
// 对于 inline 函数，如果它在头文件中定义，并在多个源文件中包含，编译器不会报重复定义的错误。这是因为 inline 函数在每个翻译单元中都被视为独立的定义，但由于它们具有相同的代码，因此在链接时会被合并
static inline vaddr_t *csr_register(word_t imm)
{
    switch (imm)
    {
    case 0x341:
        return &(cpu.csr.mepc);
    case 0x342:
        return &(cpu.csr.mcause);
    case 0x300:
        return &(cpu.csr.mstatus);
    case 0x305:
        return &(cpu.csr.mtvec);
    default:
        panic("Unknown csr");
    }
}
#define CSR(i) *csr_register(i)

// ecall
#define ECALL(dnpc, pc)                                              \
    {                                                                \
        bool success;                                                \
        dnpc = isa_raise_intr(isa_reg_str2val("$a7", &success), pc); \
    }

// mret
#define MRET(dnpc) dnpc = cpu.csr.mepc + 4 // 自陷指令的PC加4

#endif
