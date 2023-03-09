#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

static bool init_flag = false;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *exp){
  if(init_flag == false){
    init_wp_pool();
    init_flag = true;
    Log("Watchpoint pool initialized!\n");
  }
  
  if(free_ == NULL){
    panic("Error: watchpoint pool empty, no available watchpoint!\n");
  }
  
  WP *wp = free_;
  free_ = free_->next;
  strcpy(wp->expr, exp);
  wp->next = NULL;
  
  bool success = true;
  printf("%s\n", exp);
  wp->value = expr(wp->expr, &success);
  if(success == false)
    panic("Error: wrong expression!\n");
  
  if(head == NULL)
    head = wp;
  else{
    WP *p = head;
    while(p->next != NULL)
      p = p->next;
    p->next = wp;
  }
  printf("Watchpoint No.%d:%s has been set!\n", wp->NO, wp->expr);
  return wp;
}
   
void free_wp(int n){
  WP *p = head;
  if(p == NULL)
    panic("Error: watchpoint pool is empty!\n");
  
  while(p->next->NO != n){
    if(p->next == NULL)
      panic("Error: no such watchpoint!\n");
    p = p->next;
  }
  
  WP *wp_n = p->next;
  p->next = wp_n->next;
  wp_n->next = free_;
  free_ = wp_n;
  printf("Watchpoint No.%d:%s has been deleted!\n", wp_n->NO, wp_n->expr);
  return;
}
  
int check_wp(){
  if(head != NULL){
    WP *p = head;
    while(p != NULL){
      bool success = true;
      uint32_t new_value = expr(p->expr, &success);
      if(success && (new_value != p->value)){
        printf("Watchpoint No.%d's value has changed from %d to %d\n", p->NO, p->value, new_value);
        p->value = new_value;
        return 1;
      }
      p = p->next;
    }
  }
  return 0;
} 
  
void print_wp(){
  WP *p = head;
  if(p == NULL){
    printf("No watchpoint!\n");
  }
  else{
    while(p != NULL){
      printf("No.%d\t%s\t%08x\n", p->NO, p->expr, p->value);
      p = p->next;
    }
  }
  return;
}



