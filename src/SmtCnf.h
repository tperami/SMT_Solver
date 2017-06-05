#ifndef SMTCNF_H
#define SMTCNF_H

#include <vector>
#include <istream>

using SMTvaluation = const std::vector<int>&;

struct SmtCnf{
    struct Literal{
        enum Type{
            EQUAL, NOTEQ
        };
        Type type;
        int var1;
        int var2;
        bool eval(SMTvaluation val);
        bool operator()(SMTvaluation val){return eval(val);}
    };

    struct Clause{
        std::vector<Clause> literals;
        bool eval(SMTvaluation val);
        bool operator()(SMTvaluation val){return eval(val);}
    };

    int _numVar;
    std::vector<Clause> clauses;
    explicit SmtCnf(std::istream& in);
    bool eval(SMTvaluation val);
    bool operator()(SMTvaluation val){return eval(val);}
};


#endif
