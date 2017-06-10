#include <cassert>
#include "SatCnf.h"
#include <istream>
#include <limits>
#include <iostream>
#include "prettyprint.hpp"

size_t SatCnf::parserLine;

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


SatCnf::SatCnf(std::istream& in){
    parserLine = 1;
    try{
        while (in.peek() == 'c'){
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ++ parserLine;
        }
    }
    catch (istream::failure& e){
        throw SatSyntaxErr(parserLine,
                           "Input failed before p line");
    }

    try{
        if(in.peek() != 'p') in.setstate(istream::failbit);
    }
    catch (istream::failure& e){
        throw SatSyntaxErr(parserLine,
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
        throw SatSyntaxErr(parserLine, "p line has wrong format");
    }

    try{
        if(s != "cnf") in.setstate(istream::failbit);
    }
    catch (istream::failure& e){
        throw SatSyntaxErr(parserLine, "Only cnf format supported");
    }
    try{
        while(in.peek() != '\n' and in.peek() != EOF){
            in.ignore();
        }
        in.ignore();
    }
    catch (istream::failure& e){
        throw SatSyntaxErr(parserLine, "p line doesn't end on \\n");
    }

    clauses.reserve(size);
    ++parserLine;
    for(size_t i = 0 ; i < size ; ++i){
        Clause cl;
        in >> cl;
        clauses.push_back(move(cl));
        ++ parserLine;
    }
}


SatCnf::SatCnf(int nVar) : _numVar(nVar){
}

std::istream& operator>>(std::istream& in, SatCnf::Literal& lit){
    try{
        in >> lit.var;
        if(lit.var < 0){
            lit.var = -lit.var;
            lit.neg = true;
        }
        else lit.neg = false;
        lit.var--;
    }
    catch (istream::failure& e){
        throw SatCnf::SatSyntaxErr(SatCnf::parserLine,
                                   "Fail to read value of literal");
    }

    return in;
}

std::istream& operator>>(std::istream& in, SatCnf::Clause& cl){
    cl = SatCnf::Clause();
    while(in.peek() == ' '){
        in.ignore();
    }
    while(in.peek() != '\n'){
        SatCnf::Literal lit;
        in >> lit;
        cl.literals.push_back(move(lit));
        if(in.eof()) goto end;
        while(in.peek() == ' '){
            in.ignore();
        }
        if(!in.good()) goto end;
    }
    in.ignore(1);
end:
    try{
        if(cl.literals.back().var != -1)  in.setstate(istream::failbit);
    }
    catch (istream::failure& e){
        throw SatCnf::SatSyntaxErr(SatCnf::parserLine,
                                   "Clause does not end on 0");
    }
    cout << "lits :" << cl.literals << endl;
    cl.literals.pop_back();
    return in;
}

std::istream& operator>>(std::istream& in, SatCnf& Satcnf){
    Satcnf.~SatCnf();
    new(&Satcnf) SatCnf(in);
    return in;
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
