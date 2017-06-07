#ifndef SMTSOLVER_H
#define SMTSOLVER_H

#include <utility>
#include <vector>
#include "SatCnf.h"
#include "SmtCnf.h"


// generate a Sat formula and var i of the sat formula is the literal in the vector
std::pair<std::vector<SmtCnf::Literal>,SatCnf> gene(const SmtCnf& sc);

// decide if a list of literal or they opposed version is satisfiable.
// empty vector if not.
// TODO incremental i.e functor ?
std::vector<int> decide(std::vector<SmtCnf::Literal> lits,std::vector<bool> vals);

// solve a SMT CNF
// empty vector if not satisfiable
std::vector<int> solve(const SmtCnf& sc);


#endif
