/**
 * HIT365 Assignment 2 code
 * Copyright 2016 Samuel Walladge
 *
 * Tested with GNU gcc 5.3.0 and Visual Studio 2015 compiler
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
#define LINE_BUFFER 256

// all possible token types for expressions (supported operators, etc.)
// the order of things here is crafted so that the following is true:
// - all operators are > IS_OPERATOR
// - unary operators are > IS_UNARY
// - higher precedence equals higher value (ie. MULTIPLY > ADD)
typedef enum token_type {
  NOTHING,       // placeholder
  LITERAL,       // literal value (a number)
  RIGHT_PARENS,
  END_TERM,      // term deliminators < this
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

// list of operator precedences
// - where prec[token_type] == precedence of the token_type
// - zero precedence here means undefined (not an operator)
const int precedence[] = {0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 0, 3, 3};

// structure of a token for the expression parser
typedef struct token {
  token_type type;
  double value;
} token;

// structure for a linked list of tokens
typedef struct linked_list {
  token t;                    // the data
  bool isfull;                // true if the data is set
  struct linked_list * next;  // pointer to next object in the list
} linked_list;


// declare functions
token add_token(linked_list * tokens_head, token t);
linked_list * convert_rpn(const linked_list * const token_list);
void display_help(void);
double * evaluate_rpn(const linked_list * const rpn_tokens);
void free_linked_list(linked_list * thelist);
int main(void);
linked_list * new_linked_list(void);
token * pop_head(linked_list * tokens_head);
void print_linked_list(const linked_list * const tokens_head);
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

  // setup variables
  bool running = true;
  char line[LINE_BUFFER];
  char * command = NULL;
  double * memory = NULL;
  double * last_answer = NULL;
  double answer = 0;
  bool calc_success = true;
  unsigned int decimals = 0; // storing number of decimal points to print

  while (running) {

    decimals = 6; // set to 6 decimal points by default

    // prompt for a command/expression
    printf("exp>> ");
    fgets(line, LINE_BUFFER, stdin);
    command = strip(line); // strip whitespace

    // convert to lowercase
    for(size_t i=0; i<strlen(command); ++i) {
      command[i] = tolower(command[i]);
    }

    if (*command == '\0') {
      // ignore empty line

    } else if (strcmp(command, "help") == 0) {
      display_help(); // usage info

    } else if (strcmp(command, "exit") == 0) {
      running = false; // shutdown

    } else if (strcmp(command, "memory") == 0) {
      // retrieve and display memory if available
      if (memory != NULL) {
        if (*memory == floor(*memory)) {
          decimals = 0;
        }
        printf("memory = %.*f\n", decimals, *memory);
      } else {
        puts("Memory is empty!");
      }

    } else if (strcmp(command, "ans") == 0) {
      // retrieve and display last answer if available
      if (last_answer != NULL) {
        if (*last_answer == floor(*last_answer)) {
          decimals = 0;
        }
        printf("ans = %.*f\n", decimals, *last_answer);
      } else {
        puts("No previous calculations!");
      }

    } else if (strcmp(command, "store") == 0) {
      // save last answer in memory
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
      // remove memory and last answer
      free(memory); memory = NULL;
      last_answer = NULL; // don't free this - points to variable `answer`
      puts("Reset!");

    } else {
      // treat as math expression - lets calculate!
      calc_success = process_expression(command, last_answer, memory, &answer);
      // if was successful, store as last answer and print it
      if (calc_success) {
        last_answer = &answer;
        if (answer == floor(answer)) {
          decimals = 0;
        }
        printf("ans = %.*f\n", decimals, answer);
      }
    }

    free(command);
    puts(""); // just an extra line between commands for neatness

  }

  // exit the program
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

  // print_linked_list(tokens_head);

  // CONVERT TO RPN FORM
  // returns either valid rpn linked list of tokens, or NULL
  // prints errors relating to parsing the tokens
  linked_list * rpn_tokens = convert_rpn(tokens_head);
  free_linked_list(tokens_head);
  if (rpn_tokens == NULL) {
    return false;
  }

  // print_linked_list(rpn_tokens);

  // EVALUATE EXPRESSION
  // returns the answer as a pointer to a double, or NULL if failed
  // prints errors relating to evaluating the expression
  double * returned_answer = evaluate_rpn(rpn_tokens);
  free_linked_list(rpn_tokens);
  if (returned_answer == NULL) {
    return false;
  }

  // finally save the answer and return success!
  *answer = *returned_answer;
  free(returned_answer);
  return true;
}

// takes a linked_list and returns a new linked list with the tokens now in RPN
// uses the shunting yard algorithm
// returns NULL and prints error message if failed
linked_list * convert_rpn(const linked_list * const token_list) {

  // setup variables
  linked_list * output_queue = new_linked_list();
  linked_list * operator_stack = new_linked_list();

  const linked_list * current_token = token_list;
  token * temptoken = NULL;
  token_type last_type = NOTHING;
  bool first = true;

  // loop over each token in the list
  while (true) {
    if (current_token->t.type >= IS_OPERATOR) {
      // handle operators
      if (current_token->t.type >= IS_UNARY && 
          ((last_type > END_TERM && last_type < IS_UNARY) || first)) {
        puts("Missing number before unary operator!");
        free_linked_list(output_queue); free_linked_list(operator_stack);
        return NULL;
      }
      while (operator_stack->isfull) {
        // check precedence for whether to pop off the stack or not
        if (precedence[current_token->t.type]
            <= precedence[operator_stack->t.type]) {
          temptoken = pop_head(operator_stack);
          queue(output_queue, *temptoken);
          free(temptoken);
        } else {
          break;
        }
      }
      last_type = current_token->t.type;
      stack_push(operator_stack, current_token->t);

    } else if (current_token->t.type == LEFT_PARENS) {
      // remember when we find a left parens...
      stack_push(operator_stack, current_token->t);
      last_type = LEFT_PARENS;

    } else if (current_token->t.type == RIGHT_PARENS) {
      // ... so we can do fancy stuff when there's a right parens
      while (true) {
        temptoken = pop_head(operator_stack);
        if (temptoken == NULL) {
          puts("Mismatched parentheses!");
          return NULL;
        } else if (temptoken->type == LEFT_PARENS) {
          free(temptoken);
          if (last_type == LEFT_PARENS) {
            puts("Empty parentheses!");
            return NULL;
          }
          break;
        } else {
          queue(output_queue, *temptoken);
          free(temptoken);
          last_type = IS_OPERATOR;
        }
      }
      last_type = RIGHT_PARENS;

    } else {
      // handle numbers
      if (last_type <= END_TERM && !first) {
        puts("Missing operator!");
        free_linked_list(output_queue); free_linked_list(operator_stack);
        return NULL;
      }
      // numbers go straight to the output queue
      queue(output_queue, current_token->t);
      last_type = LITERAL;
    }


    if (current_token->next == NULL) { 
      // reached end - grab remaining operators and add to the queue
      token * t = NULL;
      t = pop_head(operator_stack);
      while (t != NULL) {
        // check for mismatched parentheses
        if ((*t).type == LEFT_PARENS || (*t).type == RIGHT_PARENS) {
          puts("Mismatched parentheses!");
          free(t);
          free_linked_list(output_queue); free_linked_list(operator_stack);
          return NULL;
        }
        queue(output_queue, *t);
        free(t);
        t = pop_head(operator_stack);
      }
      // pop off all operators into queue
      break; // end of list, leave the loop
    } else {
      current_token = current_token->next;
      if (first) {
        first = false;
      }
    }
  }

  free_linked_list(operator_stack);
  return output_queue;
}


// splits the expression into tokens (a makeshift linked list structure)
// returns a pointer to the list, or NULL if tokenizing failed
linked_list * tokenize(char exp[], const double * const last_answer,
                       const double * const memory) {

  // setup variables
  linked_list * tokens_head = new_linked_list();
  bool first = true; // true while at first token
  token previous = {NOTHING, 0};

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
    }
  }

  size_t i = 0;
  size_t len = strlen(exp);
  bool grab_number = false;

  // loop over each character in the expression to build the list of tokens
  while (i < len) {
    switch (exp[i]) {
      case ' ':
        break; // ignore whitespace
      // for + and -, check if previous token was the end of a term
      //  - (right parenthesis or number, or unary operator)
      case '+':
        // uses enum ordering to match correct types
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
            puts("Unrecognized variable name!");
            free_linked_list(tokens_head);
            return NULL;
          }

        } else if (isdigit(exp[i]) || (exp[i] == '.' && isdigit(exp[i+1]))) {
          grab_number = true; // tell it to parse number later
        } else {
          puts("Unrecognized character!");
          free_linked_list(tokens_head);
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
        free_linked_list(tokens_head);
        return NULL;
      }
      previous = add_token(tokens_head, (token) {LITERAL, value});
      i = i + (l-1); // advance i by number of characters gobbled
    }

    ++i;
    first = false;
  }

  return tokens_head;
}


// evaluates a linked list of tokens in rpn/postfix order
// returns a pointer to the answer if successful, else null pointer
double * evaluate_rpn(const linked_list * const rpn_tokens) {

  // setup variables
  token * left = NULL;
  token * right = NULL;
  double temp_answer = 0;
  linked_list * answer_stack = new_linked_list();
  const linked_list * current_token = rpn_tokens;
  token_type type = NOTHING;

  // loop over each token in the list
  while (true) {
    type = current_token->t.type;

    if (type >= IS_OPERATOR) {
      // pop values off the answer stack
      right = pop_head(answer_stack); // get right first - stack is in reverse
      if (right == NULL) {
        puts("Missing number for operator!");
        free_linked_list(answer_stack);
        return NULL;
      }
      if (type < IS_UNARY) { // get a second value if not a unary operator
        left = pop_head(answer_stack);
        if (left == NULL) {
          puts("Missing number for operator!");
          free(right); free_linked_list(answer_stack);
          return NULL;
        }
      }

      // check type of token and perform operation as required
      // pick up eval errors here too
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
            free(right); free(left); free_linked_list(answer_stack);
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
            free(right); free(left); free_linked_list(answer_stack);
            return NULL;
          }
          temp_answer = sqrt(right->value);
          break;
        default: // this should never happen if tokenizer worked
          puts("Encountered token with invalid type!");
          free(right); free(left); free_linked_list(answer_stack);
          return NULL;
      }

      free(left); free(right);

      // push the result back on the stack
      stack_push(answer_stack, (token) {LITERAL, temp_answer});

    } else {
      // not an operator - push to answer_stack
      stack_push(answer_stack, current_token->t);
    }

    if (current_token->next == NULL) {
      break; // end of linked list
    } else {
      current_token = current_token->next;
    }
  }

  // last token in the answer stack should be final result
  token * final = pop_head(answer_stack);

  // make sure no more elements in the answer stack and we have a final answer
  if (final == NULL || answer_stack->next != NULL || answer_stack->isfull) {
    puts("Too many numbers! (missing operator?)");
    free(final); free_linked_list(answer_stack);
    return NULL;
  }

  // return a pointer to the answer!
  double * answer = malloc(sizeof(double));;
  *answer = final->value;
  free(final); free_linked_list(answer_stack);
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
      linked_list * temp = tokens_head->next;
      tokens_head->isfull = tokens_head->next->isfull;
      tokens_head->t = tokens_head->next->t;
      tokens_head->next = tokens_head->next->next;
      free(temp); // only want to free this part of the list
    } else {
      tokens_head->isfull = false;
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

// create a new empty linked list to use
// returns a pointer to the new linked list
linked_list * new_linked_list(void) {
  linked_list * l = malloc(sizeof(linked_list));
  l->isfull = false;
  l->next = NULL;
  l->t = (token) {NOTHING, 0};
  return l;
}

void free_linked_list(linked_list * thelist) {
  linked_list * next = thelist;
  linked_list * temp = NULL;
  while (next != NULL) {
    temp = next;
    next = next->next;
    free(temp);
  }
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
    *out = '\0';
    return out;
  }

  const char * end = s + strlen(s) - 1;
  while(isspace(*end)) { // strip trailing spaces
    --end;
  }

  // Copy trimmed string and add null terminator
  size_t len = (end + 2) - s;
  out = malloc(len * sizeof(char));
  memcpy(out, s, len);
  out[len-1] = 0;

  return out;
}

// pretty print a linked_list of tokens - for debugging purposes
void print_linked_list(const linked_list * const tokens_head) {
  const linked_list * current_token = tokens_head;
  printf("[");
  while (true) {
    if (current_token->t.type >= IS_OPERATOR) {
      printf("op %d", current_token->t.type);
    } else {
      printf("val %.6f", current_token->t.value);
    }

    if (current_token->next == NULL) {
      puts("]");
      break; // end of linked list
    } else {
      printf(", ");
      current_token = current_token->next;
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

