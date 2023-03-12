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

void new_wp(char *exp){
  if(init_flag == false){
    init_wp_pool();
    init_flag = true;
    Log("Watchpoint pool initialized!\n");
  }
  
  if(free_ == NULL){
    printf("Error: watchpoint pool empty, no available watchpoint!\n");
    return;
  }
  
  WP *wp = free_;
  strcpy(wp->expr, exp);
  bool success = true;
  wp->value = expr(wp->expr, &success);
  if(success == false){
    printf("Error: wrong expression!\n");
    return;
  }
  
  free_ = free_->next;
  wp->next = NULL;
  
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
  if(p == NULL) {
    printf("Error: watchpoint pool is empty!\n");
    return;
  }
  
  if(p->NO == n) {
    head = p->next;
    p->next = free_->next;
    free_->next = p;
    printf("Watchpoint No.%d:%s has been deleted!\n", p->NO, p->expr);
    return;
  }
  
  while(p->next != NULL && p->next->NO != n) {
    p = p->next;
  }
  // p->next is the n or p(head) is the n.
  
  if(p == head && p->NO == n) {
    head = NULL;
    p->next = free_->next;
    free_->next = p;
    printf("Watchpoint No.%d:%s has been deleted!\n", p->NO, p->expr);
    return;
  }
  else if(p->next != NULL && p->next->NO == n) {
    WP *d_p = p->next;
    p->next = d_p->next;
    d_p->next = free_->next;
    free_->next = d_p;
    printf("Watchpoint No.%d:%s has been deleted!\n", d_p->NO, d_p->expr);
    return;
  }
  else {
    printf("Error: no such watchpoint!\n");
    return;
  }
}
  
bool check_wp(){
  bool changed = false;
  if(head != NULL){
    WP *p = head;
    while(p != NULL){
      bool success = true;
      uint32_t new_value = expr(p->expr, &success);
      if(success == false)
        panic("Error: wrong token!\n");
      
      if(new_value != p->value){
        Log("Watchpoint No.%d:%s's value has changed from %d to %d\n", p->NO, p->expr, p->value, new_value);
        p->value = new_value;
        changed = true;
      }
      p = p->next;
    }
  }
  return changed;
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



