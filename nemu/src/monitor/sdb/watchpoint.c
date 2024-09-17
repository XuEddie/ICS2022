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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool()
{
  int i;
  for (i = 0; i < NR_WP; i++)
  {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;     // 使用中
  free_ = wp_pool; // 空闲
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp(char *args, bool *success)
{
  assert(free_ != NULL);

  WP *wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;
  strcpy(wp->str, args);
  wp->val = expr(wp->str, success);

  return wp;
}

WP *find_wp(int NO, bool *success)
{
  if (head == NULL)
  {
    *success = false;
    return NULL;
  }
  else
  {
    WP *wp = head;
    while (wp != NULL && wp->NO != NO)
    {
      wp = wp->next;
    }
    if (wp != NULL)
    {
      *success = true;
      return wp;
    }
    else
    {
      *success = false;
      return NULL;
    }
  }
}

void free_wp(int NO, bool *success)
{
  WP *wp = find_wp(NO, success);
  if (*success)
  {
    if (head == wp)
    {
      head = head->next;
    }
    else
    {
      WP *prev = head;
      while (prev != NULL && prev->next != wp)
      {
        prev = prev->next;
      }
      if (prev != NULL)
      {
        prev->next = wp->next;
      }
    }
    // 将该监视点插入到free_链表的头部
    wp->next = free_;
    free_ = wp;
  }
}

void watchpoint_display()
{
  WP *wp = head;
  while (wp != NULL)
  {
    printf("监视点 %d: %s value is Decimal: %u Hex: 0x%x\n", wp->NO, wp->str, wp->val, wp->val);
    wp = wp->next;
  }
}

bool check_watchpoint()
{
  if (head == NULL)
  {
    return false;
  }
  else
  {
    WP *wp = head;
    while (wp != NULL)
    {
      bool success;
      word_t new_val = expr(wp->str, &success);
      if (new_val != wp->val)
      {
        printf("监视点 %d 发生变化: value is Decimal: %u -> %u Hex: 0x%x -> 0x%x\n", wp->NO, wp->val, new_val, wp->val, new_val);
        wp->val = new_val;
        return true;
      }
      wp = wp->next;
    }
    return false;
  }
}