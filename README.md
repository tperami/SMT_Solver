# SMT_Solver

This is a small SMT solver

## How to build
`make` to build the executable. The Makefile is by default in release mode, replace -O2 -DNDEBUG with -g for debug mode.

## How the SAT solver is implemented

The SAT Solver is based on DDPL. The model is built incrementally.

The BCP is done with the 2 watched literals algorithm.

On conflict a resolution phase take place : we rewind the model until a decision literal that has generated the conflict is found.

Then we backjump as far as possible and set the literal in the other way back into the model with the computed unit clause alongside.

## How the SMT solver is implemented

We don't use any persistent data structure, but to improve the performance, the SMT solver tries to generate small conflict-clauses.

Experimentally, this leads to huge performance gains, because this gives to the SAT solver more precise information on the relations between the different litterals
