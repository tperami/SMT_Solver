# SMT_Solver

This is a small SMT solver

`make` to build the executable. The Makefile is by default in release mode, replace -O2 -DNDEBUG with -g for debug mode.


The SAT Solver is based on DDPL. The model is built incrementally.
The BCP is done with the 2 watched literals algorithm.
On conflict a resolution phase take place : we rewind the model until a decision literal that has generated the conflict is found.
Then we backjump as far as possible and set the literal in the other way back into the model with the computed unit clause alongside.
