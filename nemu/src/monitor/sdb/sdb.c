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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args)
{
  int steps;
  if (args == NULL)
  {
    steps = 1;
  }
  else
  {
    sscanf(args, "%d", &steps);
  }
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args)
{
  if (args == NULL)
  {
    printf("缺少参数\n");
  }
  else if (strcmp(args, "r") == 0)
  {
    isa_reg_display();
  }
  else if (strcmp(args, "w") == 0)
  {
    watchpoint_display();
  }
  else
  {
    printf("参数错误\n");
  }
  return 0;
}

static int cmd_x(char *args)
{
  if (args == NULL)
  {
    printf("缺少参数\n");
  }
  else
  {
    char *number = strtok(args, " ");
    char *vaddr = args + strlen(number) + 1;
    if (number == NULL || vaddr == NULL)
    {
      printf("指令应为 x N EXPR\n");
      return 0;
    }
    int num;
    sscanf(number, "%d", &num);

    bool success = true;
    word_t addr = expr(vaddr, &success);
    if (success)
    {
      for (int i = 0; i < num; i++)
      {
        printf("0x%08x\n", vaddr_read(addr + i * 4, 4));
      }
    }
    else
    {
      printf("表达式不合法\n");
    }
  }
  return 0;
}

static int cmd_p(char *args)
{
  if (args == NULL)
  {
    printf("缺少参数\n");
  }
  else
  {
    bool success = true;
    word_t val = expr(args, &success); // 传指针
    if (success)
    {
      printf("%s = %u(Decimal) 0x%08x(Hex)\n", args, val, val);
    }
    else
    {
      printf("表达式不合法\n");
    }
  }
  return 0;
}

static int cmd_w(char *args)
{
  if (args == NULL)
  {
    printf("缺少参数\n");
  }
  else
  {
    bool success = true;
    WP *wp = new_wp(args, &success);
    if (success)
    {
      printf("监视点 %d: %s initial value is Decimal: %u Hex: 0x%08x\n", wp->NO, wp->str, wp->val, wp->val);
    }
    else
    {
      printf("表达式不合法\n");
    }
  }
  return 0;
}

static int cmd_d(char *args)
{
  if (args == NULL)
  {
    printf("缺少参数\n");
  }
  else
  {
    bool success = true;
    free_wp(atoi(args), &success);
    if (success)
    {
      printf("已释放监视点 %d\n", atoi(args));
    }
    else
    {
      printf("不存在该监视点\n");
    }
  }
  return 0;
}

static int cmd_help(char *args);

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "让程序单步执行N条指令后暂停执行, 当N没有给出时, 缺省为1", cmd_si},
    {"info", "info r 打印寄存器状态, info w 打印监视点信息", cmd_info},
    {"x", "求出表达式EXPR的值, 将结果作为起始内存地址, 以十六进制形式输出连续的N个4字节", cmd_x},
    {"p", "求出表达式EXPR的值", cmd_p},
    {"w", "当表达式EXPR的值发生变化时, 暂停程序执行", cmd_w},
    {"d", "删除序号为N的监视点", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
