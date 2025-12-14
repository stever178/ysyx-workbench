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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_VAR,
  TK_INT,
  TK_HEX,
  TK_EQ,
  TK_NEQ,
  TK_AND,
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\$[a-zA-Z_][a-zA-Z0-9_]*\\b", TK_VAR},
    {"-?0[xX]{1}[0-9a-fA-F]+[uU]?", TK_HEX},
    {"-?[0-9]+[uU]?", TK_INT},
    {"\\+", '+'},      // plus
    {"-", '-'},        // minus
    {"\\*", '*'},
    {"/", '/'},
    {"\\(", '('},
    {"\\)", ')'},
    {"==", TK_EQ},    // equal
    {"!=", TK_NEQ},
    {"&&", TK_AND},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};
static bool flag_init = false;

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

static Token tokens[65536] __attribute__((used)) = {};
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

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE: {
          break;
        }
        case TK_INT:
        case TK_HEX:
        case '+':
        case '-':
        case '*':
        case '/':
        case '(':
        case ')': {
          tokens[nr_token].type = rules[i].token_type;
          for (int j = 0; j < substr_len; j++) {
            tokens[nr_token].str[j] = *(substr_start + j);
          }
          tokens[nr_token].str[substr_len] = 0;
          // Log("  substr is %s", tokens[nr_token].str);
          nr_token++;
          break;
        }
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

  return true;
}

/* 空格已经过滤 */
bool check_parentheses(const uint start, const uint end) {
  if (tokens[start].type != '(') {
    return false;
  }

  for (uint i = start + 1, cnt_brackets = 1; i <= end; i++) {
    if (tokens[i].type == '(') {
      cnt_brackets++;
    } else if (tokens[i].type == ')') {
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

bool is_operator(const int op) {
  return op == '+' || op == '-' || op == '*' || op == '/';
}

int get_operator_priority(int type) {
  switch (type) {
  case '+':
  case '-':
    return 2;
  case '*':
  case '/':
    return 1;
  default:
    return 0;
  }
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
    uint64_t num = str_to_num(tokens[p].str, success);
    return (word_t)num;
  } else if (check_parentheses(p, q)) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  } else {
    /* 寻找主运算符 */
    uint op_pos = -1;

    int cnt_brackets = 0;
    for (uint i = p; i <= q; i++) {
      Log("token[%u]: %s", i, tokens[i].str);

      if (tokens[i].type == '(') {
        cnt_brackets++;
      } else if (tokens[i].type == ')') {
        cnt_brackets--;

        if (cnt_brackets < 0) {
          printf("Unmatched brackets in expression.\n");
          *success = false;
          return 0;
        }
      }

      if (cnt_brackets == 0 && is_operator(tokens[i].type)) {
        if (op_pos == -1) {
          op_pos = i;
        } else {
          int stored_priority = get_operator_priority(tokens[op_pos].type);
          int current_priority = get_operator_priority(tokens[i].type);

          /* 同优先级，最后被结合的运算符才是主运算符 */
          if (stored_priority <= current_priority) {
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
    Log("        op_pos = %u, os_type is %c", op_pos, tokens[op_pos].type);

    word_t val1 = eval(p, op_pos - 1, success);
    word_t val2 = eval(op_pos + 1, q, success);
    Log("  val1 = 0x%x, os_type is %c, val2 = 0x%x", val1, tokens[op_pos].type,
        val2);

    word_t result = 0;
    switch (tokens[op_pos].type) {
    case '+':
      result = val1 + val2;
      break;
    case '-':
      result = val1 - val2;
      break;
    case '*':
      result = val1 * val2;
      break;
    case '/':
      if (val2 == 0) {
        printf("Divide by zero.\n");
        *success = false;
        return 0;
      }
      result = val1 / val2;
      break;
    default:
      printf("Found unknown op type when evaluating expression.\n");
      *success = false;
      return 0;
    }

    // if (result > BITMASK(32)) {
    //   printf("Numeric constant too large.\n");
    //   *success = false;
    //   return 0;
    // }

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

  /* Insert codes to evaluate the expression. */
  word_t result = eval(0, nr_token - 1, success);
  return result;
}
