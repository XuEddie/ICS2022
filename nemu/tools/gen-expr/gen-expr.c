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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";

static void gen_rand_blank()
{
  switch (rand() % 6)
  {
  case 0:
  case 1:
  case 2:
    break;

  case 3:
  case 4:
    strcat(buf, " ");
    break;

  case 5:
    strcat(buf, "  ");
    break;
  }
}

static void gen_num()
{
  int len = strlen(buf);
  uint32_t num = rand() % 100;
  sprintf(buf + len, "%u", num);
}

static void gen_rand_op()
{
  int len = strlen(buf);
  char ops[] = "+-*/";
  char op = ops[rand() % 4];
  sprintf(buf + len, "%c", op);
}

static void gen_rand_expr()
{
  if (strlen(buf) > 65536 - 128)
    return; // 确保缓冲区不会溢出
  switch (rand() % 3)
  {
  case 0:
    gen_num();
    gen_rand_blank();
    break;
  case 1:
    int len = strlen(buf);
    buf[len] = '(';
    buf[len + 1] = '\0';
    gen_rand_blank();
    gen_rand_expr();
    gen_rand_blank();
    len = strlen(buf);
    buf[len] = ')';
    buf[len + 1] = '\0';
    break;
  case 2:
    gen_rand_expr();
    gen_rand_blank();
    gen_rand_op();
    gen_rand_blank();
    gen_rand_expr();
    break;
  }
}

// TODO
// 1. 如何保证表达式进行无符号运算
// 2. 如何过滤求值过程中有除0行为的表达式?

int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++)
  {
    buf[0] = '\0'; // 清空缓冲区
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
