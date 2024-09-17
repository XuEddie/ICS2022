#include <stdint.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main()
{
  return _syscall_(SYS_yield, 0, 0, 0); // 具体的系统调用 yield 没有参数
  // 最后dummy执行完毕还会调用 SYS_exit
}
