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
#define LINE_BUFFER 128

// all possible token types for expressions (supported operators, etc.)
typedef enum {
  LEFT_PARENS,
  RIGHT_PARENS,
  MULTIPLY,
  DIVIDE,
  ADD,
  MINUS,
  SQR,
  SQRT,
  MEMORY,
  ANS,
  LITERAL
} token_type;

// structure of a token for the expression parser
typedef struct {
  bool is_operator;
  token_type type;
  double value;
} token;


// declare functions
int main(void);
void display_help(void);
bool process_expression(char expression[], const double * const last_answer, const double * const memory, double * const answer);
token * tokenize(char expression[]);
char * strip(const char * str);



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

    if (command == 0) {
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
      printf("memory points to %p and last_answer points to %p\n", memory, last_answer);


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

// takes 3 pointers:
//     last_answer to be used for chaining calculations
//     memory to be used where keywoard 'memory' in calculation
//     answer is where to store the answer
// returns true if successful calculation
bool process_expression(char expression[], const double * const last_answer, const double * const memory, double * const answer) {

  double a = time(NULL); // placeholder for testing


  token * tokens = tokenize(expression);

  // TODO: recognize +-*/ at beginning of string and do last answer chaining

  // TODO: - sub in last_answer and memory
  //       - parse and evaluate
  //       - save answer in `answer` if successful
  //       - return status

  *answer = a;
  return true;

}

token * tokenize(char expression[]) {

  token * tokens = malloc(2*sizeof(token));

  printf("expression: %s\n", expression);


  //// testing
  tokens[0].type = ADD;
  tokens[0].is_operator = true;
  tokens[1].type = LITERAL;
  tokens[1].value = 10.34;
  tokens[1].is_operator = false;
  tokens[2].is_operator = false;
  ////

  // TODO: loop over the expression and add discovered tokens to the arry (dynamically allocating if needed)

  return tokens;

}


// source for information on stripping whitespace here from http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char * strip(const char * s)
{
  char * out;

  while(isspace(*s)) { // strip leading spaces
    s++;
  }

  if(*s == 0)  // All spaces?
  {
    out = malloc(sizeof(char));
    out[0] = '\0';
    return out;
  }

  // Trim trailing space
  const char *end = s + strlen(s) - 1;
  while(end > s && isspace(*end)) {
    end--;
  }
  end++;

  int len = strlen(s);
  // Set output size to minimum of trimmed string length and buffer size minus 1
  size_t out_size = (end - s) < len-1 ? (end - s) : len-1;

  // Copy trimmed string and add null terminator
  out = malloc(out_size * sizeof(char));
  memcpy(out, s, out_size);
  out[out_size] = 0;

  return out;
}
