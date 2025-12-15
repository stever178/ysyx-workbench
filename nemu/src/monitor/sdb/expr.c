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

#include <isa.h>
#include <memory/paddr.h>
#include "sdb.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_REG, TK_HEX, TK_INT,
  
  TK_OR,                      // 1. ||
  TK_AND,                     // 2. &&
  TK_BITOR,                   // 3. |
  TK_XOR,                     // 4. ^
  TK_BITAND,                  // 5. &
  TK_EQ, TK_NE,               // 6. ==, !=
  TK_LE, TK_GE, TK_LT, TK_GT, // 7. <=, >=, <, >
  TK_SHL, TK_SHR,             // 8. <<, >>
  TK_ADD, TK_SUB,             // 9. +, - (binary)
  TK_MUL, TK_DIV, TK_MOD,     // 10. *, /, %
  TK_DEREF, TK_BITNOT, TK_NOT,           // 11. ! * (unary)
  TK_L_PAREN, TK_R_PAREN,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  /* Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE}, // spaces
  
  {"\\$[a-zA-Z0-9_]*\\b", TK_REG},
  {"[-+]?0[xX]{1}[0-9a-fA-F]+[uU]?", TK_HEX},
  {"[-+]?[0-9]+[uU]?", TK_INT},

  /* 按照优先级从低到高匹配运算符 */
  {"\\|\\|", TK_OR}, // 1. ||
  {"&&", TK_AND},    // 2. &&

  {"\\|", TK_BITOR}, // 3. |
  {"\\^", TK_XOR},   // 4. ^
  {"&", TK_BITAND},  // 5. &

  {"==", TK_EQ},     // 6. ==
  {"!=", TK_NE},    // 6. !=

  {"<=", TK_LE},    // 7. <=
  {">=", TK_GE},    // 7. >=
  
  {"<<", TK_SHL},   // 8. <<
  {">>", TK_SHR},   // 8. >>

  {"<", TK_LT},     // 7. <
  {">", TK_GT},     // 7. >

  {"\\+", TK_ADD},  // 9. +
  {"-", TK_SUB},    // 9. -

  {"\\*", TK_MUL},  // 10. *
  {"/", TK_DIV},    // 10. /
  {"%", TK_MOD},    // 10. %

  {"~", TK_BITNOT}, // 11. ~
  {"!", TK_NOT},    // 11. !

  {"\\(", TK_L_PAREN},
  {"\\)", TK_R_PAREN},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};
static bool flag_init = false;

int get_operator_priority(int type) {
  switch (type) {
  // 优先级1 - 逻辑或
  case TK_OR:
    return 1;

  // 优先级2 - 逻辑与
  case TK_AND:
    return 2;

  // 优先级3 - 按位或
  case TK_BITOR:
    return 3;

  // 优先级4 - 按位异或
  case TK_XOR:
    return 4;

  // 优先级5 - 按位与
  case TK_BITAND:
    return 5;

  // 优先级6 - 等于/不等于
  case TK_EQ:
  case TK_NE:
    return 6;

  // 优先级7 - 关系运算符
  case TK_LT:
  case TK_GT:
  case TK_LE:
  case TK_GE:
    return 7;

  // 优先级8 - 位移运算符
  case TK_SHL:
  case TK_SHR:
    return 8;

  // 优先级9 - 加减法
  case TK_ADD:
  case TK_SUB:
    return 9;

  // 优先级10 - 乘除取模
  case TK_MUL:
  case TK_DIV:
  case TK_MOD:
    return 10;

  // 优先级11 - 一元运算符
  case TK_DEREF:  // 解引用
  case TK_BITNOT: // 按位取反
  case TK_NOT:    // 逻辑非
    return 11;

  default:
    return 0;
  }
}

bool is_binary_op(const int op) {
  return op == TK_OR || op == TK_AND || op == TK_BITOR || op == TK_XOR ||
         op == TK_BITAND || op == TK_EQ || op == TK_NE || op == TK_LE ||
         op == TK_GE || op == TK_LT || op == TK_GT || op == TK_SHL ||
         op == TK_SHR || op == TK_ADD || op == TK_SUB || op == TK_MUL ||
         op == TK_DIV || op == TK_MOD;
}

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
  flag_init = true;
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[MAX_TOKEN_NUM] __attribute__((used)) = {};
static uint nr_token __attribute__((used)) = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
        //     rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          break;
        case TK_REG:
        case TK_HEX:
        case TK_INT:
        case TK_L_PAREN:
        case TK_R_PAREN:
        case TK_OR:
        case TK_AND:
        case TK_BITOR:
        case TK_XOR:
        case TK_BITAND:
        case TK_EQ:
        case TK_NE:
        case TK_LE:
        case TK_GE:
        case TK_LT:
        case TK_GT:
        case TK_SHL:
        case TK_SHR:
        case TK_ADD:
        case TK_SUB:
        case TK_MUL:
        case TK_DIV:
        case TK_MOD:
        case TK_BITNOT:
        case TK_NOT:
          if (nr_token == MAX_TOKEN_NUM - 1) {
            printf("The expression is too long.\n");
            return false;
          }
          tokens[nr_token].type = rules[i].token_type;

          for (int j = 0; j < substr_len; j++) {
            tokens[nr_token].str[j] = *(substr_start + j);
          }
          tokens[nr_token].str[substr_len] = 0;

          nr_token++;
          break;
        default:
          panic("Unknown token_type: %s\n", substr_start);
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  for (i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_MUL) {
      if (i == 0) {
        tokens[i].type = TK_DEREF;
      } else {
        int prev_type = tokens[i - 1].type;
        bool is_mul = prev_type == TK_REG || prev_type == TK_HEX ||
                      prev_type == TK_INT || prev_type == TK_R_PAREN;
        if (is_mul == false) {
          tokens[i].type = TK_DEREF;
        }
      }
    }
  }

  return true;
}

/* 空格已经过滤 */
bool check_parentheses(const uint start, const uint end) {
  if (tokens[start].type != TK_L_PAREN) {
    return false;
  }

  for (uint i = start + 1, cnt_brackets = 1; i <= end; i++) {
    if (tokens[i].type == TK_L_PAREN) {
      cnt_brackets++;
    } else if (tokens[i].type == TK_R_PAREN) {
      cnt_brackets--;

      if (cnt_brackets == 0) {
        return i == end;
      } else if (cnt_brackets < 0) {
        return false;
      }
    }
  }

  return false;
}

uint64_t str_to_num(char *nptr, bool *success) {
  errno = 0;
  char *end = NULL;

  uint64_t num = strtoull(nptr, &end, 0);

  *success = false;
  if (end == nptr) {
    printf("Invalid number \"%s\".\n", nptr);
    return 0;
  }
  if (*end != '\0' && *end != 'U' && *end != 'u') {
    printf("Invalid number format \"%s\".\n", nptr);
    return 0;
  }
  if (num == ULLONG_MAX && errno == ERANGE) {
    printf("Numeric constant too large.\n");
    return 0;
  }

  *success = true;
  return num;
}

word_t eval(const uint p, const uint q, bool *success) {
  if (p > q) {
    /* Bad expression */
    *success = false;
    return 0;
  } else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    uint64_t num = 0;
    switch (tokens[p].type) {
    case TK_REG:
      if (strcmp(tokens[p].str, "$0") == 0) {
        *success = true;
        num = 0;
      } else {
        num = isa_reg_str2val(tokens[p].str + 1, success);
      }
      if (*success == false) {
        printf("Invalid register name: %s.\n", tokens[p].str);
      }
      break;
    case TK_HEX:
    case TK_INT:
      num = str_to_num(tokens[p].str, success);
      break;
    default:
      printf("Invalid token type with token %s.\n", tokens[p].str);
      *success = false;
      return 0;
    }
    return (word_t)num;
  } else if (p + 1 == q) {
    /* <unary-op> <expr> */
    switch (tokens[p].type) {
    case TK_DEREF: {
      paddr_t cur_addr = (paddr_t)str_to_num(tokens[p + 1].str, success);
      word_t data = paddr_read(cur_addr, 4);
      return data;
    }
    case TK_BITNOT:
      return ~eval(p + 1, q, success);
    case TK_NOT:
      return !eval(p + 1, q, success);
    default:
      printf("Unknown unary operator: %s\n", tokens[p].str);
      *success = false;
      return 0;
    }
  } else if (check_parentheses(p, q)) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  } else {
    /* binary op */
    uint op_pos = -1;

    /* 寻找主运算符 */
    int cnt_brackets = 0;
    for (uint i = p; i <= q; i++) {
      // Log("token[%u]: %s", i, tokens[i].str);

      if (tokens[i].type == TK_L_PAREN) {
        cnt_brackets++;
      } else if (tokens[i].type == TK_R_PAREN) {
        cnt_brackets--;

        if (cnt_brackets < 0) {
          printf("Unmatched brackets in expression.\n");
          *success = false;
          return 0;
        }
      }

      if (cnt_brackets == 0 && is_binary_op(tokens[i].type)) {
        if (op_pos == -1) {
          op_pos = i;
        } else {
          int stored_priority = get_operator_priority(tokens[op_pos].type);
          int current_priority = get_operator_priority(tokens[i].type);

          /* 同优先级，最后被结合的运算符才是主运算符 */
          if (stored_priority >= current_priority) {
            op_pos = i;
          }
        }
      }
    }

    if (op_pos == -1 || cnt_brackets != 0) {
      printf("A syntax error in expression.\n");
      *success = false;
      return 0;
    }

    word_t val1 = eval(p, op_pos - 1, success);
    word_t val2 = eval(op_pos + 1, q, success);
    word_t result = 0;

    int op_type = tokens[op_pos].type;
    Log("0x%x %s 0x%x", val1, tokens[op_pos].str, val2);

    switch (op_type) {
    case TK_OR:
      result = val1 || val2;
      break;
    case TK_AND:
      result = val1 && val2;
      break;
    case TK_BITOR:
      result = val1 | val2;
      break;
    case TK_XOR:
      result = val1 ^ val2;
      break;
    case TK_BITAND:
      result = val1 & val2;
      break;
    case TK_EQ:
      result = val1 == val2;
      break;
    case TK_NE:
      result = val1 != val2;
      break;
    case TK_LE:
      result = val1 <= val2;
      break;
    case TK_GE:
      result = val1 >= val2;
      break;
    case TK_LT:
      result = val1 < val2;
      break;
    case TK_GT:
      result = val1 > val2;
      break;
    case TK_SHL:
      if (val2 >= 32) {
        result = 0;
      } else {
        result = val1 << val2;
      }
      break;
    case TK_SHR:
      if (val2 >= 32) {
        result = 0;
      } else {
        result = val1 >> val2;
      }
      break;
    case TK_ADD:
      result = val1 + val2;
      break;
    case TK_SUB:
      result = val1 - val2;
      break;
    case TK_MUL:
      result = val1 * val2;
      break;
    case TK_DIV:
      if (val2 == 0) {
        printf("Division by zero [/].\n");
        *success = false;
        return 0;
      }
      result = val1 / val2;
      break;
    case TK_MOD:
      if (val2 == 0) {
        printf("Division by zero [%%].\n");
        *success = false;
        return 0;
      }
      result = val1 % val2;
      break;
    default:
      printf("Found unknown op type when evaluating expression.\n");
      *success = false;
      return 0;
    }

    *success = true;
    return result;
  }
}

word_t expr(char *e, bool *success) {
  if (!flag_init) {
    init_regex();
  }
  *success = false;
  if (!make_token(e)) {
    return 0;
  }

  if (nr_token == 0) {
    printf("Empty expression.\n");
    *success = false;
    return 0;
  }

  word_t result = eval(0, nr_token - 1, success);
  return result;
}
