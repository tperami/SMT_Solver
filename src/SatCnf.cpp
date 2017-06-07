#include <cassert>
#include "SatCnf.h"
#include <istream>
#include <limits>
#include <iostream>

using namespace std;

bool SatCnf::Literal::eval(SATvaluation val){
    return val[var] ^ neg;
}

bool SatCnf::Clause::eval(SATvaluation val){
    for(auto c : literals){
        if(c(val)) return true;
    }
    return false;
}

bool SatCnf::eval(SATvaluation val){
    for(auto c : clauses){
        if(!c(val)) return false;
    }
    return true;
}




SatCnf::SatCnf(int nVar) : _numVar(nVar){
}


std::ostream& operator<<(std::ostream& out, const SatCnf::Literal& lit){
    if(lit.neg) out << "¬";
    out << lit.var;
    return out;
}
std::ostream& operator<<(std::ostream& out, const SatCnf::Clause& cl){
    for(const auto& lit : cl.literals){
        out << lit << " ";
    }
    out << endl;
    return out;
}
std::ostream& operator<<(std::ostream& out, const SatCnf& Satcnf){
    out << "satcnf " << Satcnf._numVar << " " << Satcnf.clauses.size() << endl;
    for(const auto& cl : Satcnf.clauses){
        out << cl;
    }
    return out;
}
