#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c)
{
  if (user_handler)
  {
    Event ev = {0};
    switch (c->mcause)
    {
    case -1:                  // -1 0xffffffff
      ev.event = EVENT_YIELD; // yield 除此之外都可以归为 syscall
      break;

    case 0:  // SYS_exit
    case 1:  // SYS_yield
    case 2:  // SYS_open
    case 3:  // SYS_read
    case 4:  // SYS_write
    case 5:  // SYS_kill
    case 6:  // SYS_getpid
    case 7:  // SYS_close
    case 8:  // SYS_lseek
    case 9:  // SYS_brk
    case 10: // SYS_fstat
    case 11: // SYS_time
    case 12: // SYS_signal
    case 13: // SYS_execve
    case 14: // SYS_fork
    case 15: // SYS_link
    case 16: // SYS_unlink
    case 17: // SYS_wait
    case 18: // SYS_times
    case 19: // SYS_gettimeofday
      ev.event = EVENT_SYSCALL;
      break;

    default:
      ev.event = EVENT_ERROR;
      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *))
{
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap)); // 保存异常入口地址 __am_asm_trap

  // register event handler
  user_handler = handler; // 保存用户回调函数

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg)
{
  return NULL;
}

void yield()
{
  asm volatile("li a7, -1; ecall"); // 将异常种类存放到a7寄存器中，以及发起自陷 $a7 = 0xffffffff
}

bool ienabled()
{
  return false;
}

void iset(bool enable)
{
}
