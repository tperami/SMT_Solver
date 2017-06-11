#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <utility>
#include <vector>
#include <set>
#include <deque>
#include <iostream>
#include "SatCnf.h"
#include "Bitset.h"
#include "prettyprint.hpp"

// this class hold the sat solver state
class SatSolver{

    struct DInt{
        bool b :1;
        int i :31;
        operator int() const {
            return *((int*)this);
        }
        bool operator <(DInt other) const {
            return int(*this) < int(other);
        }
        DInt operator !(){
            return DInt{!b,i};
        }
        explicit DInt (int i){
            *(int*)this = i;
        }
        DInt(bool nb, int ni) : b(nb), i(ni){}
    }; // 2n : var n, 2n+1 neg var n.

    struct MLit{
        DInt var;
        bool decided; // if true, this is a decision literal
        //TODO replace it with resolution clause
    };
    size_t _numVar;
    bool _verbose;
    std::vector<MLit> _model;
    Bitset _used;
    Bitset _value;
    struct Clause{
        std::vector<DInt> clause;
        size_t wl1;
        size_t wl2;
    };
    std::vector<Clause> _clauses;
    // if b is false, then it is corresponding to wl1 else to wl2.
    std::vector<std::set<DInt>> _watched;
    std::deque<DInt> _toUpdate; // list of clauses to be updated.


    bool isTrue(DInt var) const {
        return _used[var.i] and (var.b ^ _value[var.i]);
    }
    bool isFalse(DInt var) const {
        return _used[var.i] and !(var.b ^ _value[var.i]);
    }

    void printModel() const {
        for (auto mlit : _model){
            std::cout << mlit << " ";
        }
        //std::cout << " and " << _value;
        //std::cout << std::endl;
    }

    void printWatched() const {
        for(size_t i = 0 ; i < 2*_numVar ; ++ i){
            std::cout << DInt(i) << " : " << _watched[i] << std::endl;
        }
    }

    // rules
    void setVar(DInt var); // update all clauses with a var and _used and _value.
    bool decide(); // decide a unaffected var : return false on decision, true if finished (SAT).
    void unit(DInt var); // fix this var as non-decided;
    void conflict(int clause); // resolve conflict on clause, for now only backtrack. TODO resolve
    void handle(); // take care of the next element in _toUpdate, fail badly if to update is empty.

    // pretty-printing
    friend std::ostream& operator<<(std::ostream& out, SatSolver::DInt var);
    friend std::ostream& operator<<(std::ostream& out, SatSolver::MLit var);
    friend std::ostream& operator<<(std::ostream& out, SatSolver::Clause cl);
public :
    SatSolver(int numVar, bool verbose);
    //solve a sat Cnf, returns empty vector if UNSAT.
    std::vector<bool> solve(const SatCnf& sc);

    // add a SMT Conflict clause.
    void addSMTConflict(SatCnf::Clause& cl);
};

inline std::ostream& operator<<(std::ostream& out, SatSolver::DInt var){
    out << (var.b ? "¬" : "") << var.i +1;
    return out;
}

inline std::ostream& operator<<(std::ostream& out, SatSolver::MLit var){
    out << var.var << (var.decided ? "°" : "");
    return out;
}

inline std::ostream& operator<<(std::ostream& out, SatSolver::Clause cl){
    for(size_t i = 0 ; i < cl.clause.size() -1 ; ++i){
        out << cl.clause[i] << " v ";
    }
    out << cl.clause.back() << " <- " << cl.wl1 << " <- " << cl.wl2;
    return out;
}

#endif
