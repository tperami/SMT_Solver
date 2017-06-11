#ifndef SMTSOLVER_H
#define SMTSOLVER_H

#include <utility>
#include <vector>
#include <map>
#include "SatCnf.h"
#include "SmtCnf.h"

struct SmtSatKernel {
    std::map<std::pair<int, int>, int> to;
    std::vector<std::pair<int, int>> from;
};
// generate a Sat formula and var i of the sat formula is the literal in the vector
std::pair<SmtSatKernel, SatCnf> gene(const SmtCnf& sc);

// decide if a list of literal or they opposed version is satisfiable.
// if it is satisfiable, the bool is true and vector are the equivalence classes
// if it is not, the bool is false and vector is the index of the literals that form a counter-example
std::pair<bool, std::vector<int>> decide(const SmtSatKernel& ker, std::vector<bool> vals);

// solve a SMT CNF
// empty vector if not satisfiable
std::vector<int> solve(const SmtCnf& sc, bool smtVerbose=false, bool satVerbose=false);


#endif
