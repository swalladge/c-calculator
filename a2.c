/**
 * HIT365 Assignment 2 code
 * Copyright 2016 Samuel Walladge
 *
 * Tested with GNU gcc 5.3.0 (TODO: and Visual Studio 2015 compiler)
 */

// TODO: add comments where comments lacking
// TODO: (maybe) split validation/errorchecking and tokenizing
// TODO: free memory!

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
// the order of things here is crafted so that the following is true:
// - all operators are > IS_OPERATOR
// - unary operators are > IS_UNARY
// - higher precedence equals higher value (ie. MULTIPLY > ADD)
typedef enum {
  NOTHING,
  LITERAL,       // literal value (a number)
  RIGHT_PARENS,
  END_TERM,         // term deliminators < this
  LEFT_PARENS,
  IS_OPERATOR,   // math operators > this
  MINUS,
  ADD,
  DIVIDE,
  MULTIPLY,
  IS_UNARY,      // unary operators > this
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
double * evaluate_rpn(const linked_list * const rpn_tokens);
int main(void);
token * pop_head(linked_list * tokens_head);
void print_linked_list(linked_list tokens_head);
bool process_expression(char expression[], const double * const last_answer,
                        const double * const memory,
                        double * const answer);
linked_list * queue(linked_list * tokens_head, token t);
char * strip(const char * str);
linked_list * tokenize(char exp[], const double * const last_answer,
                       const double * const memory);
linked_list * stack_push(linked_list * tokens_head, token t);



////////////////////////////////////////////////////////////////////////////////
// MAIN - BEGIN THE PROGRAM!
////////////////////////////////////////////////////////////////////////////////
int main(void) {

  puts("Type \"help\" or enter a mathematical expression.");

  bool running = true;
  char line[LINE_BUFFER];
  char * command = NULL;
  double * memory = NULL;
  double * last_answer = NULL; // &last_value;
  double answer = 0;
  bool calc_success = true;
  unsigned int decimals = 0; // storing number of decimal points to print

  while (running) {

    decimals = 6; // set to 6 decimal points by default

    printf("exp>> ");
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
        if (*memory == floor(*memory)) {
          decimals = 0;
        }
        printf("memory = %.*f\n", decimals, *memory);
      } else {
        puts("Memory is empty!");
      }

    } else if (strcmp(command, "ans") == 0) {
      if (last_answer != NULL) {
        if (*last_answer == floor(*last_answer)) {
          decimals = 0;
        }
        printf("ans = %.*f\n", decimals, *last_answer);
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
        if (*memory == floor(*memory)) {
          decimals = 0;
        }
        printf("Stored %.*f to memory.\n", decimals, *memory);
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
        if (answer == floor(answer)) {
          decimals = 0;
        }
        printf("ans = %.*f\n", decimals, answer);
      }
    }

    puts(""); // just an extra line between commands

  }


  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// MAIN EXPRESSION PROCESSING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// takes 3 pointers:
//     last_answer to be used for chaining calculations
//     memory to be used where keywoard 'memory' in calculation
//     answer is where to store the answer
// returns true if successful calculation
bool process_expression(char expression[], const double * const last_answer,
                        const double * const memory, double * const answer) {

  // TOKENIZE
  // this function will return NULL, or a linked list of valid tokens
  // automatically chains the last answer if valid
  // prints errors if invalid tokens
  linked_list * tokens_head = tokenize(expression, last_answer, memory);
  if (tokens_head == NULL) {
    return false;
  }


  // CONVERT TO RPN FORM
  // returns either valid rpn linked list of tokens, or NULL
  // prints errors relating to parsing the tokens
  linked_list * rpn_tokens = convert_rpn(tokens_head);
  if (rpn_tokens == NULL) {
    return false;
  }


  // EVALUATE EXPRESSION
  // returns the answer as a pointer to a double, or NULL if failed
  // prints errors relating to evaluating the expression
  double * returned_answer = evaluate_rpn(rpn_tokens);
  if (returned_answer == NULL) {
    return false;
  }

  // finally save the answer and return success!
  *answer = *returned_answer;
  return true;
}

// takes a linked_list and returns a new linked list with the tokens now in RPN
// returns NULL and prints error message if failed
linked_list * convert_rpn(const linked_list * const token_list) {

  linked_list * output_queue = malloc(sizeof(linked_list));
  output_queue->isfull = false;
  output_queue->next = NULL;
  linked_list * operator_stack = malloc(sizeof(linked_list));
  operator_stack->isfull = false;
  operator_stack->next = NULL;

  const linked_list * current_token = token_list;
  token * temptoken = NULL;
  token_type last_type = NOTHING;
  bool first = true;
  while (true) {
    if (current_token->t.type >= IS_OPERATOR) {
      if (current_token->t.type >= IS_UNARY && 
          ((last_type > END_TERM && last_type < IS_UNARY) || first)) {
        puts("Missing number before unary operator!");
        return NULL;
      }
      while (operator_stack->isfull) {
        if (current_token->t.type <= operator_stack->t.type) {
          queue(output_queue, *pop_head(operator_stack));
        } else {
          break;
        }
      }
      last_type = IS_OPERATOR;
      stack_push(operator_stack, current_token->t);

    } else if (current_token->t.type == LEFT_PARENS) {
      stack_push(operator_stack, current_token->t);
      last_type = LEFT_PARENS;

    } else if (current_token->t.type == RIGHT_PARENS) {
      while (true) {
        temptoken = pop_head(operator_stack);
        if (temptoken == NULL) {
          puts("Mismatched parentheses!");
          return NULL;
        } else if (temptoken->type == LEFT_PARENS) {
          if (last_type == LEFT_PARENS) {
            puts("Empty parentheses!");
            return NULL;
          }
          break;
        } else {
          queue(output_queue, *temptoken);
          last_type = IS_OPERATOR;
        }
      }
      last_type = RIGHT_PARENS;

    } else {
      if (last_type <= END_TERM && !first) {
        puts("Missing operator!");
        return NULL;
      }
      queue(output_queue, current_token->t);
      last_type = LITERAL;
    }


    if (current_token->next == NULL) { // reached end - grab remaining operators
      token * t = NULL;
      t = pop_head(operator_stack);
      while (t != NULL) {
        // check for mismatched parentheses
        if ((*t).type == LEFT_PARENS || (*t).type == RIGHT_PARENS) {
          puts("Mismatched parentheses!");
          return NULL;
        }
        queue(output_queue, *t);
        t = pop_head(operator_stack);
      }
      // pop off all operators into queue
      break; // end of linked list
    } else {
      current_token = current_token->next;
      if (first) {
        first = false;
      }
    }
  }

  return output_queue;
}


// splits the expression into tokens (a makeshift linked list structure)
// returns a pointer to the list, or NULL if tokenizing failed
linked_list * tokenize(char exp[], const double * const last_answer,
                       const double * const memory) {

  linked_list * tokens_head = malloc(sizeof(linked_list));

  (*tokens_head).next = NULL;
  (*tokens_head).isfull = false;

  /* printf("expression: %s\n", exp); */

  bool first = true; // true while at first token
  token previous;

  // insert a token if there was a last answer,
  //  and the first thing is an operator.
  if (last_answer != NULL) {
    switch (exp[0]) {
      case '-':
      case '+':
      case '*':
      case '/':
      case '^':
      case '#':
        previous = add_token(tokens_head, (token) {LITERAL, *last_answer});
        first = false;
        break;
      /* default: */
      /*   // nothing */
    }
  }

  size_t i = 0;
  size_t len = strlen(exp);
  bool grab_number = false;

  while (i < len) {
    /* printf("exp[i]: %c\n", exp[i]); */
    switch (exp[i]) {
      case ' ':
        break;
      // for + and -, check if previous token was the end of a term
      //  - (right parenthesis or number, or unary operator)
      case '+':
        // uses enum ordering to matches correct types
        if (!first && (previous.type < END_TERM || previous.type > IS_UNARY)) {
          previous = add_token(tokens_head, (token) {ADD, 0});
        } else {
          grab_number = true;
        }
        break;
      case '-':
        if (!first && (previous.type < END_TERM || previous.type > IS_UNARY)) {
          previous = add_token(tokens_head, (token) {MINUS, 0});
        } else {
          grab_number = true;
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
          if (strlen(&exp[i]) >= 6 && strncmp("memory", &exp[i], 6) == 0
              && memory != NULL) {
            previous = add_token(tokens_head, (token) {LITERAL, *memory});
            i = i + 5;
          } else if (strlen(&exp[i]) >= 3 && strncmp("ans", &exp[i], 3) == 0
              && last_answer != NULL) {
            previous = add_token(tokens_head, (token) {LITERAL, *last_answer});
            i = i + 2;
          } else {
            // fail
            puts("Unrecognized variable name!");
            return NULL;
          }

        } else if (isdigit(exp[i]) || (exp[i] == '.' && isdigit(exp[i+1]))) {
          grab_number = true; // tell it to parse number later
        } else {
          /* printf("err>> %*s\n", 1+ (int) i, "^"); */
          puts("Unrecognized character!");
          return NULL;
        }
        break;
    }
    if (grab_number) { // this is here to enable different handling of + and -
      grab_number = false;
      double value = 0;
      int l = 0;
      if (sscanf(&exp[i], "%lf%n", &value, &l) != 1) {
        puts("Invalid number format!");
        return NULL;
      }
      previous = add_token(tokens_head, (token) {LITERAL, value});
      i = i + (l-1);
    }

    ++i;
    first = false;
  }


  return tokens_head;

}


// evaluates a linked list of tokens in rpn/postfix order
// returns a pointer to the answer if successful, else null pointer
double * evaluate_rpn(const linked_list * const rpn_tokens) {

  token * left = NULL;
  token * right = NULL;
  double temp_answer = 0;
  linked_list * answer_stack = malloc(sizeof(linked_list));

  linked_list current_token = *rpn_tokens;
  token_type type;
  while (true) {
    type = current_token.t.type;

    if (type >= IS_OPERATOR) {
      // get values
      right = pop_head(answer_stack);
      if (right == NULL) {
        puts("Missing number to operator!");
        return NULL;
      }
      if (type < IS_UNARY) { // get a second value if not a unary operator
        left = pop_head(answer_stack);
        if (left == NULL) {
          puts("Missing number to operator!");
          return NULL;
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
          if (right->value == 0) {
            puts("Can't divide by zero!");
            return NULL;
          }
          temp_answer = left->value / right->value;
          break;
        case SQR:
          temp_answer = pow(right->value, 2);
          break;
        case SQRT:
          if (right->value < 0) {
            puts("Square root of negative numbers not supported!");
            return NULL;
          }
          temp_answer = sqrt(right->value);
          break;
        default: // this should never happen if tokenizer worked
          puts("Encountered token with invalid type!");
          return NULL;
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

  // make sure no more elements in the answer stack and we have a final answer
  if (final == NULL || answer_stack->next != NULL || answer_stack->isfull) {
    puts("Too many numbers! (missing operator?)");
    return NULL;
  }

  double * answer = malloc(sizeof(double));;
  *answer = final->value;
  return answer;
}


////////////////////////////////////////////////////////////////////////////////
// LINKED LIST OPERATIONS
////////////////////////////////////////////////////////////////////////////////

// add to the head of a linked_list
// - returns pointer to head (although pointer does not change) for convenience
linked_list * stack_push(linked_list * tokens_head, token t) {
  if (!tokens_head->isfull) { // if not full - ie. new empty list
    tokens_head->next = NULL;
  } else {
    // move head data to new node,
    //  which is inserted between new pushed data and original head.next
    // [<tokens_head with new data>,
    //  <new_next with old tokens_head data>,
    //  <old tokens_head.next>...]
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

// returns the token added for convenience
// - mainly just wrapper around queue()
token add_token(linked_list * tokens_head, token t) {
  queue(tokens_head, t);
  return t;
}


////////////////////////////////////////////////////////////////////////////////
// SMALLER HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// strips whitespace from beginning and end of string
// source for information on stripping whitespace from:
// - http://stackoverflow.com/q/122616/
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

// print out a linked_list of tokens - for debugging purposes
void print_linked_list(linked_list tokens_head) {
  linked_list current_token = tokens_head;
  printf("[");
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
      printf(", ");
      current_token = *current_token.next;
    }
  }
}


// prints help for operating the program
void display_help(void) {
  puts("AVAILABLE COMMANDS\n"
       "===============================================\n"
       "COMMAND       FUNCTION\n"
       "exit          Exits the program.\n"
       "help          Displays this message.\n"
       "memory        Displays memory value.\n"
       "ans           Displays last answer value.\n"
       "store         Saves last answer to memory.\n"
       "reset         Reset to startup state.\n"
       "\n"
       "SUPPORTED FUNCTIONALITY\n"
       "===============================================\n"
       "OPERATOR    DESCRIPTION     SYNTAX\n"
       "+           addition        a+b\n"
       "-           subtraction     a-b\n"
       "*           multiplication  a+b\n"
       "/           division        a+b\n"
       "^           square(a)       a^ \n"
       "#           squareroot(a)   a# \n"
       "( and )     parentheses     (a)\n"
       "-----------------------------------------------\n"
       "If expression begins with an operator,\n"
       "  the previous answer will be operated on.\n"
       "'ans' and 'memory' can be used in expressions.\n"
       "===============================================");
}
