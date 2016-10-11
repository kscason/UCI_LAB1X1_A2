INTERPRETER.cpp
=============

Documentation
-------------

INTERPRETER.cpp
Interpreter for SIMPLESEM Language
UCI CSE 141
22 August 2016
Kaitlyn Cason
UCI Student ID: 20282205

COMPILATION: 
Program was compiled using C++ 4.2.1 on Mac OS X El Capitan.

Program was compiled using the command: 
g++ INTERPRETER.cpp -o INTERPRETER

Program was executed using the command:
./INTERPRETER Program#0.txt

The input.txt file is actively searched for in the folder
that the INTERPRETER executable is in (given by the
starter code).

Given Makefile (came with starter code) will also compile 
the program using the command: make

OUTPUT: 
INTERPRETER.cpp outputs non-terminals to the standard error
line by line as they are processed.

Any write(int source) prints directly to an output file of name 
Program#.S.out (this is according to starter code). Then the data
segment is also printed directly to Program#.S.out. This file
is created in the same folder as the passed in input file 
Program#.S.

I compared other .out files (held in a different folder named 
test) using the command:
diff -w Program#.S.out ../test/Program#.S.out

ERROR HANDLING: 
Some basic error handling is taken care of with syntaxError(). 

STRUCTURE:
INTERPRETER takes input statements and conducts a FETCH-INCREMENT-
EXECUTE CYCLE. In the execution, it passes the instruction to be 
parsed via each parse function (Number, Factor, Term, Expr, Jumpt, 
Jump, Set, Statement). Once parsed, the interpreter takes the
returned Statement structure and executes the instruction 
according to the instruction semantics.