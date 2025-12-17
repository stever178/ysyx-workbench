/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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

static WP wp_pool[NR_WP] = {};
static WP *used_head = NULL, *free_head = NULL;
static uint num_used = 0;
static uint num_free = NR_WP;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].enable = false;
    memset(wp_pool[i].expr_str, 0, sizeof(wp_pool[i].expr_str));
    wp_pool[i].old_value = 0;
  }

  used_head = NULL;
  free_head = wp_pool;

  num_used = 0;
  num_free = NR_WP;
}

/* Implement the functionality of watchpoint */

static CNT_TYPE add_cnt() {
  static CNT_TYPE cnt = 0;
  cnt++;
  return cnt;
}

static void check_cnt_valid() {
  // printf("[check] num_used = %d, num_free = %d\n", num_used, num_free);
  assert(num_used + num_free == NR_WP);

  assert(num_used >= 0);
  assert(num_used <= NR_WP);

  assert(num_free >= 0);
  assert(num_free <= NR_WP);
}

/* 从free_链表中返回一个空闲的监视点结构 */
WP *new_wp() {
  if (free_head == NULL) {
    printf("The watchpoint pool is full.\n");
    return NULL;
  }

  /* delete from free-head */
  WP *result = free_head;
  free_head = free_head->next;

  /* insert in used-tail */
  result->NO = add_cnt();
  result->next = NULL;
  result->enable = true;
  memset(result->expr_str, 0, sizeof(result->expr_str));
  result->old_value = 0;

  if (used_head == NULL) {
    used_head = result;
  } else {
    for (WP* item = used_head; item; item = item->next) {
      if (item->next == NULL) {
        item->next = result;
        break;
      }
    }  
  }

  num_free--;
  num_used++;
  check_cnt_valid();

  return result;
}

/* 将wp归还到free_链表中 */
void free_wp(WP *wp) { 
  if (wp == NULL) {
    return;
  }

  if (used_head == wp) {
    used_head = used_head->next;
  } else {
    for (WP* item = used_head; item->next; item = item->next) {
      if (item->next == wp) {
        item->next = wp->next;
        break;
      }
    }  
  }
  
  /* insert in free-head */
  wp->NO = 0;
  wp->next = free_head;
  wp->enable = false;
  memset(wp->expr_str, 0, sizeof(wp->expr_str));
  wp->old_value = 0;
  free_head = wp;

  num_used--;
  num_free++;
  check_cnt_valid();
}

void free_all_wp() {
  init_wp_pool();
}

void free_wp_by_num(uint32_t num) {
  WP *free_target = NULL;

  if (used_head->NO == num) {
    used_head = used_head->next;
  } else {
    for (WP *item = used_head; item->next; item = item->next) {
      if (item->next->NO == num) {
        free_target = item->next;
        item->next = free_target->next;
        break;
      }
    }
  }

  /* insert in free-head */
  if (free_target == NULL)
    return;

  free_target->NO = 0;
  free_target->next = free_head;
  free_target->enable = false;
  memset(free_target->expr_str, 0, sizeof(free_target->expr_str));
  free_target->old_value = 0;
  free_head = free_target;

  num_used--;
  num_free++;
  check_cnt_valid();
}

WP *find_wp(uint32_t num) {
  WP *result = NULL;
  for (WP *item = used_head; item != NULL; item = item->next) {
    if (item->NO == num) {
      result = item;
      break;
    }
  }
  return result;
}

void display_wp() {
  printf("Num     Type           Disp Enb Address            What\n");
  for (WP *item = used_head; item != NULL; item = item->next) {
    printf("%3u     hw watchpoint  keep %-c   Address            %s\n",
           item->NO, item->enable ? 'y' : 'n', item->expr_str);
  }
}

void scan_wp(bool *stop) {
  *stop = false;
  for (WP* item = used_head; item; item = item->next) {
    bool success;
    uint64_t cur_value = expr(item->expr_str, &success);
    if (!success) {
      printf("invalid expression: %s\n", item->expr_str);
      continue;
    }
    if (item->old_value != cur_value) {
      *stop = true;
      break;
    }
  }
}
