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

#include <assert.h>
#include <inttypes.h>
#include <macro.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// this should be enough
static char buf[65536] = {};
static uint32_t buf_len = 0;

static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

/* generate rnd in [min, max] */
uint32_t choose(uint32_t min, uint32_t max) {
  assert(min < max);
  if (min == max)
    return min;
  return min + rand() % (max - min + (max != BITMASK(32)));
}

bool stop_generate() {
  return (choose(0, BITMASK(32)) > BITMASK(16)) &&
         (buf_len <= sizeof(buf) / 100 * 90);
}

static void gen_space(uint min, uint max) {
  uint32_t space_len = choose(min, max);
  for (uint32_t i = 0; i < space_len; i++) {
    buf_len += sprintf(buf + buf_len, " ");
  }
}

static void gen_num_dec(uint32_t rnd) {
  buf_len += sprintf(buf + buf_len, "%u", rnd);
}

static void gen_num_hex(uint32_t rnd) {
  buf_len += sprintf(buf + buf_len, "0x%x", rnd);
}

static void gen_num(uint32_t min, uint32_t max) {
  gen_space(1, 2);

  uint32_t rnd = choose(min, max);

  if (buf_len > 0) {
    switch (buf[buf_len - 1]) {
    case '*':
      rnd = choose(0, BITMASK(6));
      break;
    case '/':
      rnd = choose(1, BITMASK(6));
      break;
    default:
      break;
    }
  }

  switch (choose(0, 1)) {
  case 0:
    gen_num_dec(rnd);
    break;
  case 1:
    gen_num_hex(rnd);
    break;
  }
}

static void gen_char(char c) {
  gen_space(0, 2);
  buf_len += sprintf(buf + buf_len, "%c", c);
}

static void gen_rand_op() {
  gen_space(1, 2);
  switch (choose(0, 3)) {
  case 0:
    buf_len += sprintf(buf + buf_len, "+");
    break;
  case 1:
    buf_len += sprintf(buf + buf_len, "-");
    break;
  case 2:
    buf_len += sprintf(buf + buf_len, "*");
    break;
  case 3:
    buf_len += sprintf(buf + buf_len, "/");
    break;
  }
}

/* Considering BNF definition:
<expr> ::= <number>
  | "(" <expr> ")"
  | <expr> "+" <expr>
  | <expr> "-" <expr>
  | <expr> "*" <expr>
  | <expr> "/" <expr>
*/
enum {
  ST_IDLE = 0,
  ST_NUM,
  ST_LEFT_BRACKET,
  ST_RIGHT_BRACKET,
  ST_OP,
  ST_STOP,
};

static uint8_t gstate = ST_IDLE;

static const uint32_t MAX_DEPTH = 10;

/* Output randomly generated expression to buffer buf. */
static uint32_t choice = 0;
static void gen_rand_expr(uint8_t depth) {
  switch (gstate) {
  case ST_IDLE:
  case ST_LEFT_BRACKET:
  case ST_OP:
    if (depth < MAX_DEPTH) {
      choice = choose(1, 3);
    } else {
      choice = 1;
    }
    break;
  case ST_NUM:
  case ST_RIGHT_BRACKET:
  case ST_STOP:
    choice = 0;
    break;
  }

  switch (choice) {
  case 0:
    /* unreach */
    buf[buf_len] = '\0';
    printf("[depth:%2d] choice 0\n", depth);
    return;
  case 1:
    gen_num(0, BITMASK(32));
    gstate = ST_NUM;
    break;
  case 2: 
    gen_char('(');
    gstate = ST_LEFT_BRACKET;
    gen_rand_expr(depth + 1);
    gen_char(')');
    gstate = ST_RIGHT_BRACKET;
    break;
  case 3: 
    gen_rand_expr(depth + 1);
    gen_rand_op();
    gstate = ST_OP;
    gen_rand_expr(depth + 1);
    gstate = ST_STOP;
    // printf("[depth:%2d] stop\n", depth);
    break;
  }

  // if (stop_generate()) {
  //   gstate = ST_STOP;
  // }
}

static void gen_expr() {
  gstate = ST_IDLE;
  buf_len = 0;
  gen_rand_expr(0);
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  uint32_t loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%u", &loop);
  }
  int i;
  for (i = 0; i < loop; i++) {
    gen_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Wno-overflow -Wdiv-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint32_t result;
    ret = fscanf(fp, "%u", &result);
    pclose(fp);

    /* The value EOF is returned if the end of input is reached before
    either the first successful conversion or a matching failure occurs.
    */
    if (ret == EOF) {
      continue;
    }

    printf("0x%08x %s\n", result, buf);
  }
  return 0;
}
