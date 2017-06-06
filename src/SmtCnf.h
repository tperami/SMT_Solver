#ifndef SMTCNF_H
#define SMTCNF_H

#include <vector>
#include <istream>
#include <stdexcept>
#include <string>
#include <sstream>
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

    class SmtSyntaxErr : public std::istream::failure{
        std::string get(int line, const std::string& msg){
            std::stringstream s;
            s << "Line " << line << ", SyntaxError : " << msg;
            return s.str();
        }
    public:
        SmtSyntaxErr(int line, const std::string& msg)
            : std::istream::failure(get(line, msg)){
        }
    };

    static size_t parserLine;
    int _numVar;
    std::vector<Clause> clauses;
    explicit SmtCnf(std::istream& in);
    bool eval(SMTvaluation val);
    bool operator()(SMTvaluation val){return eval(val);}
};

std::istream& operator>>(std::istream& in, SmtCnf::Literal& lit);
std::istream& operator>>(std::istream& in, SmtCnf::Clause& cl);
std::istream& operator>>(std::istream& in, SmtCnf::Clause& SmtCnf);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Literal& lit);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Clause& cl);
std::ostream& operator<<(std::ostream& out, const SmtCnf::Clause& SmtCnf);


#endif
