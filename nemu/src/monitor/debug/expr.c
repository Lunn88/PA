#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NEQ,
  TK_DEC,
  TK_HEX,
  TK_REG,
  TK_NEG,
  TK_POINT,
  TK_AND,
  TK_OR

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},		// minus
  {"\\*", '*'},		// mutiple
  {"\\/", '/'},		// divide
  {"\\%", '%'},         // mod
  {"\\(", '('},		// left parenthesis
  {"\\)", ')'},		// right parenthesis
  
  {"==", TK_EQ},	 // equal
  {"!=", TK_NEQ},	 // not equal
  {"&&", TK_AND},        // and
  {"\\|\\|", TK_OR},     // or
	
  {"[1-9][0-9]*|0", TK_DEC},		// decimal
  {"0[xX][0-9a-fA-F]+", TK_HEX},	// hex
  {"\\$[eE][a-dsA-DSi][xpiXPI]|\\$[a-dsA-DS][xpiXPI]|\\$[a-dA-D][hlHL]", TK_REG}  				       // register
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
	  case TKNOTYPE: break;

          default:
		strcpy(tokens[nr_token].str, substr_start, substr_len);
		tokens[nr_token].str[substr_len] = '\0';
		tokens[ne_token].type = rules[i].token_type;

		if(tokens[nr_token].type == '*' && (nr_token == 0 || (tokens[nr_token-1].type != TK_10 && tokens[nr_token-1].type != TK_REG && tokens[nr_token-1].type != TK_16)))
			tokens[nr_token].type = TK_POINT;
                   if(tokens[nr_token].type == '-' && (nr_token == 0 || (tokens[nr_token-1].type != TK_10 && tokens[nr_token-1].type != TK_REG && tokens[nr_token-1].type != TK_16)))
	                tokens[nr_token].type = TK_NEG;
		nr_token++;
		break;
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for(int i = 0; i < nr_token; i++)
	  Log("tokens[%d]: '%s' (%d)", i, tokens[i].str, tokens[i].type);

  /* TODO: Insert codes to evaluate the expression. */
  
  uint32_t result = eval(0, nr_token - 1);

  return result;
}


bool check_parentheses(int p ,int q){
  int tag = 0, flag = 1;
  for(int i = p; i <= q; i++){
	if(tokens[i].type == '(')
	   	tag++;
	else if(tokens[i].type == ')')
	   	tag--;
        if(tag < 0)
		panic("Error: 括号错误！\n");
	if(tag == 0 && i < q)
		flag = 0;
  }
  if(tag != 0)
	panic("Error: 括号错误！\n");
  if(flag == 0)
	return false;
  return true;
}

int dominant_operator(int p, int q) {
  int dominant = p, left = 0, flag = 0;
  for(int i = p; i <= q; i++) {
    if(tokens[i].type == '(') {
      left++;
      i++;
      while(true) {
	if(tokens[i].type == '(')
	  left++;
	else if(tokens[i].type == ')')
	  left--;
	i++;
	if(left == 0)
	  break;
      }
      if(i > q)
	break;
    }
    if(tokens[i].type == TK_DEC || tokens[i].type == TK_HEX || tokens[i].type == TK_REG)
      continue;
    if(op_precedence(tokens[i].type) >= flag) {
	    flag = op_precedence(tokens[i].type);
	    dominant = i;
	    char dest[255] = "\0";
	    for(int j = p; j <= q; j++)
		    strcat(dest, tokens[j].str);
	    Log("The dominant of %s is tokens[%d] = '%s'", dest, dominanat, tokens[dominant].str);
    }
  }
  return dominant;
}

int op_precedence(int op){
	if(op == 259 || op == 260) // "==", "!="
		return 7;
	else if(op == '+' || op == '-')
		return 4;
	else if(op == '*' || op == '/' || op == '%')
		return 3;
	else if(op == 261 || op ==263) // TK_NEG, TK_POINT
		return 2;
	else if(op == 264) // "&&"
		return 11;
	else if(op == 265) // "||"
		return 12;
	else
		return 0;
}

uint32_t eval(int p, int q) {
  if(p > q)
    return -1;
  else if (p == q) {
    // Single token

    if(tokens[p].type == TK_DEC)
      return atoi(tokens[p].str);
    
    else if(tokens[p].type == TK_HEX){
      int tmp;
      sscanf(tokens[p].str, "%x", &tmp);
      return tmp;
    }


}
