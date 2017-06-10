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

    class SatSyntaxErr : public std::istream::failure{
        std::string get(int line, const std::string& msg){
            std::stringstream s;
            s << "Line " << line << ", SyntaxError : " << msg;
            return s.str();
        }
    public:
        SatSyntaxErr(int line, const std::string& msg)
            : std::istream::failure(get(line, msg)){
        }
    };


    static size_t parserLine;
    size_t _numVar;
    std::vector<Clause> clauses;
    explicit SatCnf(std::istream& in);
    explicit SatCnf(int nVar);
    bool eval(SATvaluation val);
    bool operator()(SATvaluation val){return eval(val);}
};

std::istream& operator>>(std::istream& in, SatCnf::Literal& lit);
std::istream& operator>>(std::istream& in, SatCnf::Clause& cl);
std::istream& operator>>(std::istream& in, SatCnf& SatCnf);
std::ostream& operator<<(std::ostream& out, const SatCnf::Literal& lit);
std::ostream& operator<<(std::ostream& out, const SatCnf::Clause& cl);
std::ostream& operator<<(std::ostream& out, const SatCnf& Satcnf);


#endif
