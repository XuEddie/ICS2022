#include <common.h>

static Context *do_event(Event e, Context *c)
{
  switch (e.event)
  {
  case EVENT_NULL:
    // printf("EVENT NULL\n");
    break;
  case EVENT_YIELD:
    // printf("EVENT YIELD\n");
    break;
  case EVENT_SYSCALL:
    // printf("EVENT SYSCALL\n");
    do_syscall(c);
    break;
  case EVENT_ERROR:
  default:
    panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void)
{
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
