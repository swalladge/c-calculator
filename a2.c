/**
 * HIT365 Assignment 2 code
 * Copyright 2016 Samuel Walladge
 *
 * Tested with GNU gcc 5.3.0 (TODO: and Visual Studio 2015 compiler)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// use sscanf_s as drop-in replacement for sscanf if Visual Studio compiler
#ifdef _MSC_VER
 #define sscanf sscanf_s
#endif

// set line buffer
#define LINE_BUFFER 128

// all possible token types for expressions (supported operators, etc.)
typedef enum {
  LITERAL,
  IS_OPERATOR,
  LEFT_PARENS,
  RIGHT_PARENS,
  MULTIPLY,
  DIVIDE,
  ADD,
  MINUS,
  IS_UNARY,
  SQR,
  SQRT
} token_type;

// structure of a token for the expression parser
typedef struct token {
  token_type type;
  double value;
} token;

typedef struct linked_list {
  token t;
  bool isfull;
  struct linked_list * next;
} linked_list;


// declare functions
token add_token(linked_list * tokens_head, token t);
linked_list * convert_rpn(const linked_list * const token_list);
void display_help(void);
bool evaluate_rpn(const linked_list * const rpn_tokens, double * const answer);
int main(void);
token * pop_head(linked_list * tokens_head);
void print_linked_list(linked_list tokens_head);
bool process_expression(char expression[], const double * const last_answer, const double * const memory, double * const answer);
linked_list * queue(linked_list * tokens_head, token t);
char * strip(const char * str);
linked_list * tokenize(char exp[], const double * const last_answer, const double * const memory);
linked_list * stack_push(linked_list * tokens_head, token t);



// begin the program!
int main(void) {

  puts("Type \"help\" or enter a mathematical expression.");

  bool running = true;
  char line[LINE_BUFFER];
  char * command = NULL;
  double * memory = NULL; //malloc(sizeof(double)); // &memory_value;
  double * last_answer = NULL; // &last_value;
  double answer = 0;
  bool calc_success = true;

  while (running) {

    printf("%s", "exp>> ");
    fgets(line, LINE_BUFFER, stdin);
    command = strip(line); // strip whitespace

    // convert to lowercase
    for(size_t i=0; i<strlen(line); ++i) {
      command[i] = tolower(command[i]);
    }

    if (*command == 0) {
      // ignore empty line

    } else if (strcmp(command, "help") == 0) {
      display_help();

    } else if (strcmp(command, "exit") == 0) {
      // shutdown
      running = false;

    } else if (strcmp(command, "memory") == 0) {
      if (memory != NULL) {
        printf("memory = %.6lf\n", *memory);
      } else {
        puts("Memory is empty!");
      }

    } else if (strcmp(command, "ans") == 0) {
      if (last_answer != NULL) {
        printf("ans = %.6lf\n", *last_answer);
      } else {
        puts("No previous calculations!");
      }

    } else if (strcmp(command, "store") == 0) { // save last answer in memory
      if (last_answer == NULL) {
        puts("No previous answer to store!");
      } else {
        if (memory == NULL) {
          memory = malloc(sizeof(double));
        }
        *memory = *last_answer;
        printf("%.6lf stored to memory\n", *memory);
      }

    } else if (strcmp(command, "reset") == 0) {
      memory = NULL;
      last_answer = NULL;
      puts("Reset!");

    } else {
      // treat as math expression
      calc_success = process_expression(command, last_answer, memory, &answer);
      if (calc_success) {
        last_answer = &answer;
        /* printf("ans = %.*lg\n", (int) log10(answer)+6, answer); */
        printf("ans = %.6lf\n", answer);
      }
    }

    puts(""); // just an extra line between commands

  }


  return 0;
}

void display_help(void) {
  puts("COMMAND       FUNCTION");
  puts("exit          Exits the program.");
  puts("help          Displays this message.");
  puts("memory        Displays memory value.");
  puts("ans           Displays last answer value.");
  puts("store         Saves last answer to memory.");
  puts("reset         Wipes memory and last answer values.");
  puts("");
  puts("OPERATOR    DESCRIPTION    SYNTAX");
  puts("+           addition       [a+b|+a]");
  puts("-           subtraction    [a-b|-a]");
  puts("*           multiplication [a+b|*a]");
  puts("/           division       [a+b|/a]");
  puts("^           square(x)      [a^|^]  ");
  puts("#           squareroot(x)  [a#|#]  ");


}

void print_linked_list(linked_list tokens_head) {
  linked_list current_token = tokens_head;
  printf("%s", "[");
  while (true) {
    if (current_token.t.type >= IS_OPERATOR) {
      printf("op %d", current_token.t.type);
    } else {
      printf("val %.6f", current_token.t.value);
    }

    if (current_token.next == NULL) {
      puts("]");
      break; // end of linked list
    } else {
      printf("%s", ", ");
      current_token = *current_token.next;
    }
  }
}


// add to the head of a linked_list 
// - also returns pointer to head (although pointer does not change) for convenience
linked_list * stack_push(linked_list * tokens_head, token t) {
  if (!tokens_head->isfull) { // if not full - ie. new empty list
    tokens_head->next = NULL;
  } else {
    // move head data to new node, which is inserted between new pushed data and original head.next
    // [<tokens_head with new data>, <new_next with old tokens_head data>, <old tokens_head.next>...]
    linked_list * new_next = malloc(sizeof(linked_list));
    new_next->next = tokens_head->next;
    new_next->t = tokens_head->t;
    new_next->isfull = tokens_head->isfull;

    tokens_head->next = new_next;
  }

  tokens_head->t = t;
  tokens_head->isfull = true;
  return tokens_head;
}

// add to the end of a linked_list
// returns pointer to the head for convenience (same pointer as in arguments)
linked_list * queue(linked_list * tokens_head, token t) {
  if (!tokens_head->isfull) { // if still empty list
      tokens_head->t = t;
      tokens_head->isfull = true;
      tokens_head->next = NULL;
      return tokens_head;
  }
  linked_list * current_token = tokens_head;
  while (true) {
    if (current_token->next == NULL) {
      current_token->next = malloc(sizeof(linked_list));
      current_token->next->isfull = true;
      current_token->next->next = NULL;
      current_token->next->t = t;
      break;
    } else {
      current_token = current_token->next;
    }
  }
  return tokens_head;
}

// remove and return the first element in the linked list
token * pop_head(linked_list * tokens_head) {
  if (tokens_head->isfull) {
    token * t = malloc(sizeof(token));
    *t = tokens_head->t;
    if (tokens_head->next != NULL) {
      tokens_head->isfull = tokens_head->next->isfull;
      tokens_head->t = tokens_head->next->t;
      tokens_head->next = tokens_head->next->next;
    } else {
      tokens_head->isfull = false;
      tokens_head->next = NULL;
    }
    return t;
  } else {
    return NULL;
  }
}

linked_list * convert_rpn(const linked_list * const token_list) {

  linked_list * output_queue = malloc(sizeof(linked_list));
  output_queue->isfull = false;
  output_queue->next = NULL;
  linked_list * operator_stack = malloc(sizeof(linked_list));
  operator_stack->isfull = false;
  operator_stack->next = NULL;

  linked_list current_token = *token_list;
  while (true) {
    if (current_token.t.type >= IS_OPERATOR) {
      stack_push(operator_stack, current_token.t);
    } else {
      queue(output_queue, current_token.t);
    }

    // TODO: more processing to handle parens, operator precedence, etc.

    if (current_token.next == NULL) {
      token * t = NULL;
      t = pop_head(operator_stack);
      while (t != NULL) {
        queue(output_queue, *t);
        t = pop_head(operator_stack);
      }
      // pop off all operators into queue
      break; // end of linked list
    } else {
      current_token = *current_token.next;
    }
  }

  return output_queue;
}

// takes 3 pointers:
//     last_answer to be used for chaining calculations
//     memory to be used where keywoard 'memory' in calculation
//     answer is where to store the answer
// returns true if successful calculation
bool process_expression(char expression[], const double * const last_answer, const double * const memory, double * const answer) {

  // TOKENIZE
  bool last = false;
  if (last_answer != NULL) {
    last = true;
  }
  linked_list * tokens_head = tokenize(expression, last_answer, memory);
  if (tokens_head == NULL) {
    puts("Invalid expression!");
    return false;
  }
  /* print_linked_list(*tokens_head); */


  // CHAIN TO LAST ANSWER IF NEEDED
  if (tokens_head->t.type >= IS_OPERATOR && last_answer != NULL) {
    linked_list new_head;
    linked_list * new_tokens_head = &new_head;
    new_tokens_head->isfull = true;
    new_tokens_head->t = (token) {LITERAL, *last_answer};
    new_tokens_head->next = tokens_head;
    tokens_head = new_tokens_head;
  }
  /* print_linked_list(*tokens_head); */

  
  // CONVERT TO RPN FORM
  linked_list * rpn_tokens = convert_rpn(tokens_head);
  /* print_linked_list(*rpn_tokens); */

  // EVALUATE EXPRESSION
  bool result = evaluate_rpn(rpn_tokens, answer);

  if (!result) {
    puts("Invalid expression!");
  }
  return result;

}


// returns the token added for convenience
token add_token(linked_list * tokens_head, token t) {
  /* printf("found token: %d\n", t.type); */
  
  linked_list * current_token = tokens_head;
  while (true) {
    if (current_token->isfull) {
      if (current_token->next == NULL) {
        current_token->next = malloc(sizeof(linked_list));
      }
      current_token = current_token->next;
    } else {
      current_token->t = t;
      current_token->isfull = true;
      break;
    }
  }
  return t;
}

// splits the expression into tokens (a makeshift linked list structure)
// returns a pointer to the list, or NULL if tokenizing failed
linked_list * tokenize(char exp[], const double * const last_answer, const double * const memory) {

  linked_list * tokens_head = malloc(sizeof(linked_list));

  (*tokens_head).next = NULL;
  (*tokens_head).isfull = false;

  /* printf("expression: %s\n", exp); */

  size_t i = 0;
  size_t len = strlen(exp);

  // setup a dummy previous token to decide how to handle expressions beginning with + or -
  token previous;
  if (last_answer != NULL) {
    previous.type = LITERAL;
  } else {
    previous.type = IS_OPERATOR;
  }
  bool grab_number = false;
  while (i < len) {
    /* printf("exp[i]: %c\n", exp[i]); */
    switch (exp[i]) {
      case ' ':
        break;
      // for + and -, check if previous token was an operator or not
      //  - if was operator, then assume it begins a positive/negative number
      //  - otherwise should be operator
      case '+':;
        if (previous.type >= IS_OPERATOR) {
          // parse number
          grab_number = true;
        } else {
          previous = add_token(tokens_head, (token) {ADD, 0});
        }
        break;
      case '-':
        if (previous.type >= IS_OPERATOR) {
          // parse number (since number can begin with -)
          grab_number = true;
        } else {
          previous = add_token(tokens_head, (token) {MINUS, 0});
        }
        break;
      case '*':
        previous = add_token(tokens_head, (token) {MULTIPLY, 0});
        break;
      case '/':
        previous = add_token(tokens_head, (token) {DIVIDE, 0});
        break;
      case '^':
        previous = add_token(tokens_head, (token) {SQR, 0});
        break;
      case '#':
        previous = add_token(tokens_head, (token) {SQRT, 0});
        break;
      case '(':
        previous = add_token(tokens_head, (token) {LEFT_PARENS, 0});
        break;
      case ')':
        previous = add_token(tokens_head, (token) {RIGHT_PARENS, 0});
        break;
      default:
        if (isalpha(exp[i])) {
          // parse memory/ans/otherkeywords
          if (strlen(&exp[i]) >= 6 && strncmp("memory", &exp[i], 6) == 0 && memory != NULL) {
            previous = add_token(tokens_head, (token) {LITERAL, *memory});
            i = i + 5;
          } else if (strlen(&exp[i]) >= 3 && strncmp("ans", &exp[i], 3) == 0 && last_answer != NULL) {
            previous = add_token(tokens_head, (token) {LITERAL, *last_answer});
            i = i + 2;
          } else {
            // fail
            return NULL;
          }

        } else if (isdigit(exp[i]) || (exp[i] == '.' && isdigit(exp[i+1]))) { // allow begin with '.'
          grab_number = true; // parse number
        } else {
          return NULL;
        }
        break;
    }
    if (grab_number) { // this check down here to enable handling + or - in different ways
      grab_number = false;
      double value = 0;
      int l = 0;
      sscanf(&exp[i], "%lf%n", &value, &l);
      if (l == 0) {
        return NULL;
      }
      previous = add_token(tokens_head, (token) {LITERAL, value});
      i = i + (l-1);
    }

    ++i;
  }


  return tokens_head;

}


// source for information on stripping whitespace here from http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char * strip(const char * s) {
  char * out;

  while(isspace(*s)) { // strip leading spaces
    ++s;
  }

  if(*s == 0) { // was it all spaces?
    out = malloc(sizeof(char));
    out[0] = '\0';
    return out;
  }

  const char *end = s + strlen(s) - 1;
  while(end > s && isspace(*end)) { // strip trailing spaces
    --end;
  }
  ++end;

  int len = strlen(s);
  // Set output size to minimum of trimmed string length and buffer size minus 1
  size_t out_size = (end - s) < len-1 ? (end - s) : len-1;

  // Copy trimmed string and add null terminator
  out = malloc(out_size * sizeof(char));
  memcpy(out, s, out_size);
  out[out_size] = 0;

  return out;
}


// evaluates a linked list of tokens in rpn/postfix order
// sets answer to the result of the evaluation, returns whether successful or not
bool evaluate_rpn(const linked_list * const rpn_tokens, double * const answer) {

  token * left = NULL;
  token * right = NULL;
  double temp_answer;
  linked_list * answer_stack = malloc(sizeof(linked_list));

  linked_list current_token = *rpn_tokens;
  token_type type;
  while (true) {
    type = current_token.t.type;

    if (type >= IS_OPERATOR) {
      // get values
      right = pop_head(answer_stack);
      if (right == NULL) {
        return false;
      }
      if (type < IS_UNARY) { // get a second value if not a unary operator
        left = pop_head(answer_stack);
        if (left == NULL) {
          return false;
        }
      }

      // check type of token and perform operation as required
      switch (type) {
        case ADD:
          temp_answer = left->value + right->value;
          break;
        case MINUS:
          temp_answer = left->value - right->value;
          break;
        case MULTIPLY:
          temp_answer = left->value * right->value;
          break;
        case DIVIDE:
          temp_answer = left->value / right->value;
          break;
        case SQR:
          temp_answer = pow(left->value, 2);
          break;
        case SQRT:
          temp_answer = sqrt(left->value);
          break;
        default:
          return false; // this should never happen
      }

      stack_push(answer_stack, (token) {LITERAL, temp_answer});

    } else { // not an operator - push to answer_stack
      stack_push(answer_stack, current_token.t);
    }

    if (current_token.next == NULL) {
      break; // end of linked list
    } else {
      current_token = *current_token.next;
    }
  }

  token * final = pop_head(answer_stack);

  // make sure no more elements in the answer stack and that we have a final answer
  if (final == NULL || answer_stack->next != NULL || answer_stack->isfull) {
    return false;
  }
  
  *answer = final->value;

  return true;

}
