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

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

extern const char *regs[];

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc)
{
  if (ref_r->pc != pc)
  {
    printf("PC expected %x but got %x\n", ref_r->pc, pc);
    return false;
  }
  for (int i = 0; i < 32; i++)
  {
    if (ref_r->gpr[i] != cpu.gpr[i])
    {
      printf("对于[%s]寄存器, expceted 0x%08x, but got 0x%08x.\n", regs[i], ref_r->gpr[i], cpu.gpr[i]);
      printf("ref-pc = 0x%08x\n", ref_r->pc);
      printf("dut-pc = 0x%08x\n", pc);
      return false;
    }
  }
  return true;
}

void isa_difftest_attach()
{
}
