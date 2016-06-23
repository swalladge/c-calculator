
# C Calculator

A simple calculator in C for HIT365 Assignment 2.

Copyright © 2016 Samuel Walladge


## Background

This was written as an assignment for a university subject on C programming.  The program implements a simple math
calculator with several features as detailed below. It uses dynamic data structures, a tokenizer, the shunting yard
algorithm to convert to RPN, and an RPN evaluator.


## Features

- Basic calculator operators including multiplication, division, addition, subtraction, and parentheses for grouping.
- Square and square root functions.
- Stores last answer for use in next calculation.
- Supports chaining expression on to previous answer (when there is a previous answer and first thing in the expression is an operator).
- Memory function.
- Respects standard math order of operations.
- Quick in-program help.
- Source code uses only standard C library functions - portable.
- Tested on Linux with GCC 5.3.0 and on Windows with Visual Studio (2015 and 2013 versions).


## Example Session

```
$ make && ./a2.x86_64
gcc a2.c -o a2.`uname -m` -lm
Type "help" or enter a mathematical expression.

exp>> help
AVAILABLE COMMANDS
===============================================
COMMAND       FUNCTION
exit          Exits the program.
help          Displays this message.
memory        Displays memory value.
ans           Displays last answer value.
store         Saves last answer to memory.
reset         Reset to startup state.

SUPPORTED FUNCTIONALITY
===============================================
OPERATOR    DESCRIPTION     SYNTAX
+           addition        a+b
-           subtraction     a-b
*           multiplication  a+b
/           division        a+b
^           square(a)       a^ 
#           squareroot(a)   a# 
( and )     parentheses     (a)
-----------------------------------------------
- If expression begins with an operator,
   the previous answer will be operated on.
- 'ans' and 'memory' can be used in expressions.
- Standard order of operations are respected.
-----------------------------------------------
Example: (-42+4*10)^ + memory
===============================================

exp>> 6*9
ans = 54

exp>> +6
ans = 60

exp>> store
Stored 60 to memory.

exp>> (-50+10/2)+100
ans = 55

exp>> +memory^
ans = 3655

exp>> /3000
ans = 1.218333

exp>> 3  
ans = 3

exp>> *ans*ans
ans = 27

exp>> memory
memory = 60

exp>> ans                                                                
ans = 27

exp>> reset
Reset!

exp>> memory
Memory is empty!

exp>> 5/0
Can't divide by zero!

exp>> ((2/3)
Mismatched parentheses!

exp>> exit
$
```
