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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include <memory/vaddr.h>

enum
{
  TK_NOTYPE = 256,
  TK_EQ,

  /* TODO: Add more token types */
  TK_DECIMAL,
  TK_HEX,
  TK_REG,
  TK_UEQ,
  TK_DEREF,
  TK_MINUS,
  TK_AND,
};

static struct rule
{
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */
    {"0x[0-9a-fA-F]+", TK_HEX}, // HEX
    {"\\$[0-9a-z]+", TK_REG},   // REG
    {"[0-9]+", TK_DECIMAL},     // DECIMAL
    {" +", TK_NOTYPE},          // spaces
    {"\\+", '+'},               // plus
    {"\\-", '-'},               // sub
    {"\\*", '*'},               // mul
    {"\\/", '/'},               // divide
    {"==", TK_EQ},              // equal
    {"!=", TK_UEQ},             // unequal
    {"\\(", '('},               // 左括号
    {"\\)", ')'},               // 右括号
    {"&&", TK_AND},             // and

};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[256] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type)
        {
        case TK_NOTYPE:
          // Do nothing for spaces
          break;

        case '-':
          if (nr_token == 0 || !(tokens[nr_token - 1].type == ')' || tokens[nr_token - 1].type == TK_DECIMAL || tokens[nr_token - 1].type == TK_HEX || tokens[nr_token - 1].type == TK_REG))
          {
            tokens[nr_token].type = TK_MINUS;
          }
          nr_token++;
          break;

        case '*':
          if (nr_token == 0 || !(tokens[nr_token - 1].type == ')' || tokens[nr_token - 1].type == TK_DECIMAL || tokens[nr_token - 1].type == TK_HEX || tokens[nr_token - 1].type == TK_REG))
          {
            tokens[nr_token].type = TK_DEREF; // 给定地址，解引用
          }
          nr_token++;
          break;

        case TK_DECIMAL:
        case TK_HEX:
        case TK_REG:
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;

        default:
          nr_token++;
          break;
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q)
{
  if (tokens[p].type == '(' && tokens[q].type == ')')
  {
    int count = 0;
    for (int i = p; i <= q; i++)
    {
      if (tokens[i].type == '(')
      {
        count++;
      }
      if (tokens[i].type == ')')
      {
        count--;
      }
      if (count == 0 && i != q)
      {
        return false;
      }
    }
    return count == 0;
  }
  else
  {
    return false;
  }
}

int get_operator_priority(int type)
{
  switch (type)
  {
  case TK_AND:
    return 1;

  case TK_EQ:
  case TK_UEQ:
    return 2;

  case '+':
  case '-':
    return 3;

  case '*':
  case '/':
    return 4;

  default:
    return 100;
  }
}

int find_main_operator(int p, int q)
{
  int op = -1;
  int min_priority = 100;
  int parentheses = 0;

  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(')
    {
      parentheses++;
    }
    else if (tokens[i].type == ')')
    {
      parentheses--;
    }
    else if (parentheses == 0)
    {
      int priority = get_operator_priority(tokens[i].type);
      if (priority <= min_priority)
      {
        min_priority = priority;
        op = i;
      }
    }
  }
  return op;
}

word_t eval(int p, int q, bool *success)
{
  if (p > q)
  {
    *success = false;
    return 0;
  }
  else if (p == q)
  {
    if (tokens[p].type == TK_DECIMAL)
    {
      *success = true;
      return atoi(tokens[p].str);
    }
    else if (tokens[p].type == TK_HEX)
    {
      *success = true;
      word_t val;
      sscanf(tokens[p].str, "%x", &val);
      return val;
    }
    else if (tokens[p].type == TK_REG)
    {
      bool flag = true;
      word_t val = isa_reg_str2val(tokens[p].str, &flag);
      if (flag)
      {
        *success = true;
        return val;
      }
      else
      {
        *success = false;
        printf("不存在该寄存器\n");
        return 0;
      }
    }
  }
  else if (check_parentheses(p, q) == true)
  {
    return eval(p + 1, q - 1, success);
  }
  else if (tokens[p].type == TK_MINUS)
  {
    return -eval(p + 1, q, success);
  }
  else if (tokens[p].type == TK_DEREF)
  {
    return vaddr_read(eval(p + 1, q, success), 4);
  }
  else
  {
    int op = find_main_operator(p, q);
    if (op == -1)
    {
      *success = false;
      return 0;
    }
    word_t val1 = eval(p, op - 1, success);
    if (*success == false)
    {
      return 0;
    }
    word_t val2 = eval(op + 1, q, success);
    if (*success == false)
    {
      return 0;
    }

    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
      if (val2 == 0)
      {
        printf("除0异常\n");
        *success = false;
        return 0;
      }
      return val1 / val2;
    case TK_EQ:
      return val1 == val2;
    case TK_UEQ:
      return val1 != val2;
    case TK_AND:
      return val1 && val2;
    default:
      assert(0);
    }
  }
  return 0;
}

word_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  word_t val = eval(0, nr_token - 1, success);

  return val;
}
