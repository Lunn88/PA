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
  char *arg = strtok(NULL," ");
  int steps = 0;
  if(arg==NULL){
    cpu_exec(1);  //one step
    return 0;
  }
  sscanf(arg,"%d",&steps);
  if(steps<-1){
    printf("Error Integer!\n");
    return 0;
  }
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  char *arg=strtok(NULL," ");
  if(strcmp(arg,"r") == 0){
  //info r
    for(int i=0;i<8;i++)
      printf("%s \t0x%x \t%d\n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
      printf("eip \t0x%x \t%d\n",cpu.eip,cpu.eip);
  }
  else if(strcmp(arg,"w") == 0){
  //info w
    print_wp();
  }
  return 0;
}

static int cmd_x(char *args){
  char *N = strtok(NULL," ");
  char *EXPR = strtok(NULL," ");
  int len;

  len=atoi(N);

  bool flag = true;
  vaddr_t address = expr(EXPR, &flag);
  if (!flag){
    printf("Error: wrong expr!\n");
    return 0;
  }

  for(int i=0;i<len;i++){
    uint32_t data = vaddr_read(address+i*4,4);
    printf("0x%08x\t",address+i*4);
    for(int j=0;j<4;j++){
      printf("0x%02x\t",data&0xff);
      data=data>>8;
    }
    printf("\n");
  }
  return 0;
}

static int cmd_p(char *args){
	bool success = true;
	if(args == NULL){
		printf("Error: missing arguments!\n");
	    return 0;
	}
	uint32_t result = expr(args, &success);
	if(success)
	  printf("Expresssion: %s = %d\n", args, result);
	else
	  printf("Error: wrong expression!\n");
    return 0;
}

static int cmd_w(char *args){
  if(args == NULL){
    printf("Error: missing arguments!\n");
    return 0;
  }
  new_wp(args);
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
  {"si", "Single ", cmd_si},
  {"info","INFO",cmd_info},
  {"x","Scan memory",cmd_x},
  {"p","Expr Cal",cmd_p},
  { "w" , "Set watchpoint", cmd_w},
  { "d" , "Delete watchpoint", cmd_d}
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
