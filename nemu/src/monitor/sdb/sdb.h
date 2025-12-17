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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

#define MAX_TOKEN_NUM 65536

uint64_t str_to_num(char *nptr, bool *success);
word_t expr(char *e, bool *success);

#define NR_WP 32

typedef uint32_t CNT_TYPE;

typedef struct watchpoint {
  CNT_TYPE NO;
  struct watchpoint *next;
  
  // int type;
  bool enable;
  char expr_str[MAX_TOKEN_NUM];
  word_t old_value;

  /* TODO: Add more members if necessary */
} WP;

void init_wp_pool();

WP* new_wp();
void free_wp(WP *wp);
void free_all_wp();
void free_wp_by_num(uint32_t num);

WP *find_wp(uint32_t num);
void display_wp();

void scan_wp(bool *stop);

#endif
