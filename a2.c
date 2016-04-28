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

// use sscanf_s as drop-in replacement for sscanf if Visual Studio compiler
#ifdef _MSC_VER
 #define sscanf sscanf_s
#endif

// set line buffer
#define LINE_BUFFER 128

// variable to store a line of data entered by the user


// declare functions
int main(void);
void display_help(void);
bool process_expression(double * last_answer, double * memory, double * answer);

// begin the program!
int main(void) {

  puts("Type \"help\" or enter a mathematical expression.");

  bool running = true;
  char line[LINE_BUFFER];
  char command[LINE_BUFFER];
  double * memory = NULL; //malloc(sizeof(double)); // &memory_value;
  double * last_answer = NULL; // &last_value;
  double answer = 0;
  bool calc_success = true;
  int was_command;

  while (running) {

    printf("%s", "exp>> ");
    fgets(line, LINE_BUFFER, stdin);
#ifdef _MSC_VER
    was_command = sscanf_s(line, "%s", command, LINE_BUFFER); // VS compiler wants a length as extra argument
#else
    was_command = sscanf(line, "%s", command);
#endif

    if (was_command < 0) {
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
      calc_success = process_expression(last_answer, memory, &answer);
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
bool process_expression(double * last_answer, double * memory, double * answer) {

  double a = time(NULL); // placeholder for testing
  // TODO: parse and calculate!

  *answer = a;
  return true;

}
