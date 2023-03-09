#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  if(args == NULL){
	  cpu_exec(1);
  }
  else{
	  cpu_exec(atoi(args));
  }
  return 0;
}

static int cmd_info(char *args) {
  if(args == NULL) {
    printf("Error: missing arguments!\n");
    return 0;
  }
    
  switch(*args) {
	  case 'r':
		  printf("EAX : %x\n", cpu.eax);
		  printf("EBX : %x\n", cpu.ebx);
		  printf("ECX : %x\n", cpu.ecx);
		  printf("EDX : %x\n", cpu.edx);
		  printf("ESP : %x\n", cpu.esp);
		  printf("EBP : %x\n", cpu.ebp);
		  printf("ESI : %x\n", cpu.esi); 
		  printf("EDI : %x\n", cpu.edi);
		  break;
	  case 'w':
		  print_wp();
		  break;
	  default: printf("Error: wrong input!\n");
  }
  return 0;
}

static int cmd_x(char* args) {
  char *arg = strtok(args, " ");
  
  if(arg == NULL){
    printf("Error: missing argument n!\n");
    return 0;
  }
  
  int n = atoi(arg);
  char *EXPR = strtok(NULL, " ");
  if(EXPR == NULL){
    printf("Error: missing argument addr!\n");
    return 0;
  }
  
  bool success = true;
  vaddr_t addr = expr(EXPR, &success);
  if (success == false){
    printf("Error: wrong expr!\n");
    return 0;
  }
  
  for(int i = 0; i < n; i++){
    uint32_t data = vaddr_read(addr + i * 4, 4);
    printf("0x%08x\t", addr + i * 4);
    for(int j = 0; j < 4; j++){
      printf("0x%02x\t" , data & 0xff);
      data = data >> 8;
    }
    printf("\n");
  }
return 0;
}

static int cmd_w(char *args){
  if(args == NULL){
    printf("Error: missing arguments!\n");
    return 0;
  }
  WP *wp = new_wp(args);
  return 0;
}

static int cmd_d(char *args){
  if(args == NULL){
    printf("Error: missing arguments!\n");
    return 0;
  }
  int n = atoi(args);
  free_wp(n);
  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute n instructions and then stop", cmd_si },
  { "info", "Print program state", cmd_info },
  { "x" , "Print len memory start from addr", cmd_x },
  { "w" , "Set watchpoint", cmd_w},
  { "d" , "Delete watchpoint", cmd_d},
  
  /* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
