#include <cassert>
#include "SmtCnf.h"


bool SmtCnf::Literal::eval(SMTvaluation val){
    switch(type){
        case EQUAL:
            return val[var1] == val[var2];
        case NOTEQ:
            return val[var1] != val[var2];

    }
    assert(false);
}

bool SmtCnf::Clause::eval(SMTvaluation val){
    for(auto c : literals){
        if(c(val)) return true;
    }
    return false;
}

bool SmtCnf::eval(SMTvaluation val){
    for(auto c : clauses){
        if(!c(val)) return false;
    }
    return true;
}
