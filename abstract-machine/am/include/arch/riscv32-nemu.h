#ifndef ARCH_H__
#define ARCH_H__

struct Context
{
  // TODO: fix the order of these members to match trap.S
  // uintptr_t mepc, mcause, gpr[32], mstatus;
  uintptr_t gpr[32]; // 0 * XLEN 到 31 * XLEN
  uintptr_t mcause;  // 32 * XLEN
  uintptr_t mstatus; // 33 * XLEN
  uintptr_t mepc;    // 34 * XLEN
  void *pdir;        // 35 * XLEN
};

#define GPR1 gpr[17] // a7 系统调用号
#define GPR2 gpr[10] // a0 参数
#define GPR3 gpr[11] // a1 参数
#define GPR4 gpr[12] // a2 参数
#define GPRx gpr[10] // a0 返回值

#endif
