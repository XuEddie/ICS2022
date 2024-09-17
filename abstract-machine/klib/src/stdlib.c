#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void)
{
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed)
{
  next = seed;
}

int abs(int x)
{
  return (x < 0 ? -x : x);
}

int atoi(const char *nptr)
{
  int x = 0;
  while (*nptr == ' ')
  {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9')
  {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

// static char *addr = NULL;

static char *start_addr;   // addr的初始值
static bool init_flag = 0; // 初始化的标志, 初始化完成后置1

void *malloc(size_t size)
{
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()

  // #if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  //   // 如果 addr 尚未初始化，设置其为堆的起始位置
  //   if (addr == NULL)
  //   {
  //     addr = heap.start;
  //   }

  //   // 对 size 进行对齐操作，使其满足8字节对齐要求
  //   size = (size_t)((size + 7) & ~7);

  //   // 保存当前的分配起始地址
  //   char *old_addr = addr;

  //   // 更新 addr 以指向下一个可分配空间的开始位置
  //   addr += size;

  //   // 确保分配的内存块在堆的范围内
  //   assert((uintptr_t)heap.start <= (uintptr_t)addr && (uintptr_t)addr <= (uintptr_t)heap.end);

  //   // 返回旧地址，也就是这次分配的起始地址
  //   return old_addr;
  // #endif
  //   return NULL;

  if (!init_flag)
  {
    start_addr = (void *)ROUNDUP(heap.start, 8);
    init_flag = true;
  }
  size = (size_t)ROUNDUP(size, 8);
  char *old = start_addr; // 获取addr
  start_addr += size;
  return old; // [addr, addr + size]
}

void free(void *ptr)
{
}

#endif
