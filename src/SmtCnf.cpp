#include <cassert>
#include "SmtCnf.h"
#include <istream>
#include <limits>

using namespace std;

size_t SmtCnf::parserLine;

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




SmtCnf::SmtCnf(std::istream& in){
    parserLine = 1;
    try{
        while (in.peek() == 'c'){
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ++ parserLine;
        }
    }
    catch (istream::failure& e){
        throw SmtSyntaxErr(parserLine,
                           "Input failed before p line");
    }

    try{
        if(in.peek() != 'p') in.setstate(istream::failbit);
    }
    catch (istream::failure& e){
        throw SmtSyntaxErr(parserLine,
                           "Begin of line is not p or c before first p");
    }

    if(!in) return;
    // reading p line
    string s;
    size_t size;
    try{
        in.ignore(1);
        in >> s;
        in >> _numVar;
        in >> size;
    }
    catch(istream::failure&){
        throw SmtSyntaxErr(parserLine, "p line has wrong format");
    }

    try{
        if(s != "cnf") in.setstate(istream::failbit);
    }
    catch (istream::failure& e){
        throw SmtSyntaxErr(parserLine, "Only cnf format supported");
    }

    clauses.reserve(size);
    ++parserLine;
    for(size_t i = 0 ; i < size ; ++i){
        Clause cl;
        in >> cl;
        ++ parserLine;
    }
}



std::istream& operator>>(std::istream& in, SmtCnf::Literal& lit){
    try{
        in >> lit.var1;
    }
    catch (istream::failure& e){
        throw SmtCnf::SmtSyntaxErr(SmtCnf::parserLine,
                                   "Fail to read first variable of literal");
    }

    try{
        switch (in.peek()){
            case '=':
                lit.type = SmtCnf::Literal::EQUAL;
                in.ignore(1);
                break;
            case '<':
                in.ignore(1);
                if(in.get() != '>') in.setstate(istream::failbit);
                else lit.type = SmtCnf::Literal::NOTEQ;
                break;
            default:
                in.setstate(istream::failbit);
        }
    }
    catch (istream::failure& e){
        throw SmtCnf::SmtSyntaxErr(SmtCnf::parserLine,
                                   "Comparison symbol invalid while reading literal");
    }

    try{
        in >> lit.var2;
    }
    catch (istream::failure& e){
        throw SmtCnf::SmtSyntaxErr(SmtCnf::parserLine,
                                   "Fail to read second variable of literal");
    }

    return in;
}

std::istream& operator>>(std::istream& in, SmtCnf::Clause& cl){
    cl = SmtCnf::Clause();
    if(in.eof()) return in;
    in >> ws;
    if(in.eof()) return in;
    while(in.peek() != '\n'){
        SmtCnf::Literal lit;
        in >> lit;
        if(in.eof()) return in;
        in >> ws;
        if(!in.good()) return in;
    }
    in.ignore(1);
    return in;
}

std::istream& operator>>(std::istream& in, SmtCnf::Clause& SmtCnf);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Literal& lit);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Clause& cl);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Clause& SmtCnf);
