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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);

void test_expr();

int main(int argc, char *argv[])
{
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Test expr */
  // test_expr();

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}

void test_expr()
{
  // 打开指定的input文件
  FILE *fp = fopen("/home/utensil/ics2022/nemu/tools/gen-expr/input", "r");
  if (fp == NULL)
  {
    perror("fopen");
  }

  char line[65536];
  while (fgets(line, sizeof(line), fp))
  {
    unsigned expected_result;
    char expression[65536];
    if (sscanf(line, "%u %[^\n]", &expected_result, expression) != 2)
    {
      fprintf(stderr, "Invalid input format: %s", line);
      continue;
    }

    bool success = true;
    unsigned actual_result = expr(expression, &success);

    if (!success)
    {
      printf("Error evaluating expression: %s\n", expression);
    }
    else if (expected_result != actual_result)
    {
      printf("Mismatch: expected %u, got %u for expression: %s\n",
             expected_result, actual_result, expression);
    }
    else
    {
      printf("Match: %u %s\n", actual_result, expression);
    }
  }

  fclose(fp);
}