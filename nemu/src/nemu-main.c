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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
uint64_t str_to_num(char *nptr, bool *success);
word_t expr(char *, bool *);

int main(int argc, char *argv[]) {
  int test_pos = 2;
  if (argc > test_pos && strcmp(argv[test_pos], "-t") == 0) {
    FILE *fp = fopen(argv[test_pos], "r");
    assert(fp != NULL);
    
    char result_str[128];
    char expr_str[65536];
    char line[65536];

    bool success = false;
    uint cnt = 0;

    // difficult to distinguish newlines from other white space
    // while(fscanf(fp, "%s %s", result_str, expr_str) == 2) {

    while (fgets(line, sizeof(line), fp) != NULL) {
      sscanf(line, "%s %[^\n]", result_str, expr_str);
      printf("[[#%5u]] %s = %s\n", cnt, result_str, expr_str);

      uint64_t expect = str_to_num(result_str, &success);
      if (!success) {
        printf("[[#%5u]] expected num failed\n", cnt);
        assert(0);
      }

      word_t result = expr(expr_str, &success);
      if (success) {
        if (result != (word_t)expect) {
          printf("[[#%5u]] expect 0x%x ; result 0x%x\n", cnt, (word_t)expect, result);
          assert(0);
        }
      } else {
        printf("[[#%5u]] failed\n", cnt);
        assert(0);
      }
      
      cnt ++;
    }

    fclose(fp);
    return 0;
  }

  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
