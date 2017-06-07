#ifndef SATCNF_H
#define SATCNF_H

#include <vector>
#include <istream>
#include <stdexcept>
#include <string>
#include <sstream>


using SATvaluation = const std::vector<bool>&;

struct SatCnf{
    struct Literal{
        bool neg;
        int var;
        bool eval(SATvaluation val);
        bool operator()(SATvaluation val){return eval(val);}
    };

    struct Clause{
        std::vector<Literal> literals;
        bool eval(SATvaluation val);
        bool operator()(SATvaluation val){return eval(val);}
    };

    int _numVar;
    std::vector<Clause> clauses;
    SatCnf(int nVar);
    bool eval(SATvaluation val);
    bool operator()(SATvaluation val){return eval(val);}
};

std::ostream& operator<<(std::ostream& out, const SatCnf::Literal& lit);
std::ostream& operator<<(std::ostream& out, const SatCnf::Clause& cl);
std::ostream& operator<<(std::ostream& out, const SatCnf& Satcnf);


#endif
