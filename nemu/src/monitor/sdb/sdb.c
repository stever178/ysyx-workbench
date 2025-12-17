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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;
static char* delimiter = " ";

void init_regex();

static char *line_read = NULL;
static char line_history[MAX_TOKEN_NUM + 1] = {0};

/* We use the `readline` library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    if (strlen(line_read) > MAX_TOKEN_NUM) {
      printf("The expression is too long.\n");
      return NULL;
    }
    strcpy(line_history, line_read);
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

#define NR_SUBCMD ARRLEN(info_subcmd_table)

/* q */
static int cmd_q(char *args) {
  bool flag_quit = true;

  while (nemu_state.state == NEMU_RUNNING || nemu_state.state == NEMU_STOP) {
    printf("A running session is active.\n");

    if (line_read) {
      free(line_read);
      line_read = NULL;
    }
    line_read = readline("Quit anyway? (y or n) ");

    if (*line_read == 'y') { 
      break;
    } else if (*line_read == 'n') {
      flag_quit = false;
      break;
    } else {
      printf("Please answer y or n.\n");
    }
  }

  if (flag_quit) {
    nemu_state.state = NEMU_QUIT;
    printf("Exit NEMU.\n");
    return -1;
  } else {
    return 0;
  }
}

/* si [N] */
static int cmd_si(char *args) {
  if (args == NULL) {
    cpu_exec(1);
    return 0;
  }

  char *args_end = args + strlen(args);

  /* split */
  char *arg = strtok(args, delimiter);
  if (arg == NULL) {
    cpu_exec(1);
    return 0;
  }

  char *arg_tail = arg + strlen(arg) + 1;
  if (arg_tail < args_end) {
    char *tmp = strtok(arg_tail, delimiter);
    if (tmp != NULL) {
      printf("Ambiguous si command \"%s%s%s\".\n",
        arg, delimiter, arg_tail);
      return 0;
    }
  }

  /* check isdigit */
  int length = strlen(arg);
  for (int i = 0; i < length; i++) {
    if (!isdigit((unsigned char)arg[i])) {
      printf("Invalid number \"%s\".\n", arg);
      return 0;
    }
  }

  bool success = false;
  uint64_t num = str_to_num(arg, &success);
  if (!success) {
    return 0;
  }
  
  cpu_exec(num);
  return 0;
}

static struct {
  const char *name;
  const char *description;
  void (*handler) ();
} info_subcmd_table [] = {
  { "r", "Display registers", isa_reg_display },
  { "w", "Display watchpoints", isa_watchpoint_display },
  /* TODO: Add more subcommands */
};

/* info SUBCMD */
static int cmd_info(char *args) {
  if (args == NULL) {
    printf("info command cannot be empty.\n");
    return 0;
  }
  
  char *args_end = args + strlen(args);

  char *arg = strtok(args, delimiter);
  if (arg == NULL) {
    printf("info command cannot be empty.\n");
    return 0;
  }

  char *arg_tail = arg + strlen(arg) + 1;
  if (arg_tail < args_end) {
    char *tmp = strtok(arg_tail, delimiter);
    if (tmp != NULL) {
      printf("Ambiguous info command \"%s%s%s\".\n",
             arg, delimiter, arg_tail);
      return 0;
    }
  }

  int i;
  for (i = 0; i < NR_SUBCMD; i++) {
    if (strcmp(arg, info_subcmd_table[i].name) == 0) {
      info_subcmd_table[i].handler();
      break;
    }
  }
  if (i == NR_SUBCMD) {
    printf("Ambiguous info command \"%s\".\n", arg);
  }
  return 0;
}

/* x N EXPR */
static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Argument required (starting display address).\n");
    return 0;
  }

  char *args_end = args + strlen(args);

  /* split */
  char *n_str = strtok(args, delimiter);
  if (n_str == NULL) {
    printf("Argument required (starting display address).\n");
    return 0;
  }

  /* check isdigit */
  int len = strlen(n_str);
  for (int i = 0; i < len; i++) {
    if (!isdigit((unsigned char)n_str[i])) {
      printf("Invalid number \"%s\".\n", n_str);
      return 0;
    }
  }

  /* N */
  bool success = false;
  uint64_t num = str_to_num(n_str, &success);
  if (!success) {
    return 0;
  }

  if (num == 0) {
    printf("The number of bytes to examine must be greater than 0.\n");
    return 0;
  }

  /* EXPR */
  char *expr_str = n_str + strlen(n_str) + 1;
  if (expr_str >= args_end) {
    printf("Expression is required for x command.\n");
    return 0;
  }

  success = false;
  paddr_t base_addr = (paddr_t)expr(expr_str, &success);
  if (!success) {
    return 0;
  }

  /* print */
  int byte_num = 4;
  for (uint64_t i = 0; i < num; i++) {
    paddr_t cur_addr = base_addr + i * byte_num;
    word_t data = paddr_read(cur_addr, byte_num);
    printf("0x%08" PRIx32 ": 0x%08" PRIx32 "\n", cur_addr, data);
  }

  return 0;
}

static uint32_t cnt_p = 0;
static char p_history_str[MAX_TOKEN_NUM + 1] = {};

/* p EXPR */
static int cmd_p(char *args) {
  char *expr_str = NULL;

  if (args == NULL) {
    if (cnt_p == 0) {
      printf("The history is empty.\n");
      return 0;
    } else {
      expr_str = p_history_str;
    }

  } else {
    if (strlen(args) > MAX_TOKEN_NUM) {
      printf("The expression is too long.\n");
      return 0;
    }
    expr_str = args;
    if (expr_str == NULL) {
      if (cnt_p == 0) {
        printf("The history is empty.\n");
        return 0;
      } else {
        expr_str = p_history_str;
      }
    }
  }

  bool success = false;
  word_t result = expr(expr_str, &success);
  if (!success) {
    return 0;
  }
  strcpy(p_history_str, expr_str);
  cnt_p++;

  printf(ANSI_FMT("$%u", ANSI_FG_BLUE) " = %" PRIu32 "\n", cnt_p, result);

  return 0;
}

/* w EXPR */
static int cmd_w(char *args) {
  char *expr_str = NULL;

  if (args == NULL) {
    printf("Argument required (expression to compute).\n");
    return 0;
  } else {
    if (strlen(args) > MAX_TOKEN_NUM) {
      printf("The expression is too long.\n");
      return 0;
    }
    expr_str = args;
  }

  bool success = false;
  word_t result = expr(expr_str, &success);
  if (!success) {
    return 0;
  }

  WP *wp = new_wp();
  if (wp == NULL) {
    return 0;
  }

  strcpy(wp->expr_str, expr_str);
  wp->old_value = result;

  printf("Hardware watchpoint %u: %s\n", wp->NO, expr_str);

  return 0;
}

/* d N */
static int cmd_d(char *args) {
  while (args == NULL) {
    if (line_read) {
      free(line_read);
      line_read = NULL;
    }
    line_read = readline("Delete all breakpoints, watchpoints, tracepoints, "
                         "and catchpoints? (y or n) \n");

    if (*line_read == 'y') {
      free_all_wp();
      return 0;
    } else if (*line_read == 'n') {
      break;
    } else {
      printf("Please answer y or n.\n");
    }
  }

  if (strlen(args) > MAX_TOKEN_NUM) {
    printf("The expression is too long.\n");
    return 0;
  }

  char *num_str = args;
  bool success = false;
  word_t num = expr(num_str, &success);
  if (!success) {
    return 0;
  }

  free_wp_by_num(num);

  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands.", cmd_help },
  { "c", "Continue the execution of the program.", cmd_c },
  { "q", "Exit NEMU.", cmd_q },
  { "si", "Step one instruction.", cmd_si },
  { "info", "Display information about the current state of the program.", cmd_info },
  { "x", "Scan memory.", cmd_x },
  { "p", "Evaluate expression.", cmd_p },
  { "w", "Set up monitoring points.", cmd_w },
  { "d", "Delete monitoring points.", cmd_d },
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command \"%s\".\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  char str_bak[MAX_TOKEN_NUM + 1] = {0};
  for (char *str; (str = rl_gets()) != NULL;) {
    if (*str == '\0') {
      strcpy(str_bak, line_history);
      str = str_bak;
    }
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, delimiter);
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command \"%s\".\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
