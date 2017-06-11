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

// This class hold the sat solver state
// The class invariants are programmatically stated in checkInvariant();
class SatSolver{

    /**
      This struct is the fusion of an integer and a boolean

      There are 2 uses :
          - variable : then the boolean means the negation of the variable
          - clause watch point : then the boolean design the watch point
            false for 1 and true for 2.
     */
    struct DInt{
        bool b :1;
        int i :31;
        operator int() const {
            return *((int*)this);
        }
        // Warning : some pieces of code use the fact that the integer is compared
        // before the boolean
        bool operator <(DInt other) const {
            return int(*this) < int(other);
        }
        DInt operator !(){
            return DInt(!b,i);
        }
        explicit DInt (int i){
            *(int*)this = i;
        }
        DInt(bool nb, int ni) : b(nb), i(ni){}
        DInt(){assert(false);}
    };

    /*
      This struct represent a literal in the model.

      The variable is var (it can be negated).
      if this is a decision literal then &decidingCl == nullptr;
      else decidingCl is the clause that lead to this decision.
      toDelete is designating if the clause must be deleted on literal destruction.

      Warning : constructor overloading is strange : if a pointer is given it will destroy it
      but it will not if it's a reference.
     */
    struct MLit{
        DInt var;
        // if decision == {} : the var has been decided else this is the deciding clause.
        std::vector<DInt>& decidingCl; // sorted array
        bool toDelete = false;
        MLit(DInt v, std::vector<DInt>* dCl) : var(v), decidingCl(*dCl), toDelete(true){}
        MLit(DInt v, std::vector<DInt>& dCl) : var(v), decidingCl(dCl), toDelete(false){}
        MLit(MLit&& oth) : var(oth.var), decidingCl(oth.decidingCl), toDelete(oth.toDelete){
            oth.toDelete = false;
        }
        ~MLit(){
            if(toDelete) delete &decidingCl;
        }
        // HACK (this function is never used)
        MLit() : decidingCl(*(std::vector<DInt>*)nullptr){assert(false);}
    };
    // The number of variable
    size_t _numVar;
    // Enable verbose mode
    bool _verbose;
    // Current model M
    std::vector<MLit> _model;
    Bitset _used; // set of variable in the model;
    Bitset _value; // value of variable in the model, the value is undefined if not in the model.

    // This is a solver clause with its 2 watching value.
    struct Clause{
        std::vector<DInt> clause;
        size_t wl1;
        size_t wl2;
    };

    // This is the list of clauses.
    std::vector<Clause> _clauses;

    // list of clause to be rechecked on setting a variable to false.
    // has size 2*_numVar and is indexed by the conversion to int of the literal DInt.
    std::vector<std::set<DInt> > _watched;
    // list of clauses to be updated.
    std::deque<DInt> _toUpdate;


    // Check if a var is true in the current model.
    bool isTrue(DInt var) const {
        return _used[var.i] and (var.b ^ _value[var.i]);
    }
    // Check if a var is false in the current model.
    bool isFalse(DInt var) const {
        return _used[var.i] and !(var.b ^ _value[var.i]);
    }

    // print the current model.
    void printModel() const {
        for (auto& mlit : _model){
            std::cout << mlit << " ";
        }
    }

    // print the state of _watched.
    void printWatched() const {
        for(size_t i = 0 ; i < 2*_numVar ; ++ i){
            std::cout << DInt(i) << " : " << _watched[i] << std::endl;
        }
    }

    // merge two clauses (during resolution phase).
    static void fusion (std::vector<DInt>& target, const std::vector<DInt>& other){
        const std::vector<DInt> ori = std::move(target);
        target.clear();
        target.reserve(ori.size() + other.size());
        size_t orip = 0, othp = 0;
        while (orip < ori.size() or othp < other.size()){
            if(orip == ori.size()){
                target.insert(target.end(),other.begin() + othp,other.end());
                break;
            }
            else if(othp == other.size()){
                target.insert(target.end(),ori.begin() + orip,ori.end());
                break;
            }
            else {
                if (ori[orip].i < other[othp].i){
                    target.push_back(ori[orip]);
                    orip++;
                }
                else if (ori[orip].i > other[othp].i){
                    target.push_back(other[othp]);
                    othp++;
                }
                else{
                    if(ori[orip].b == other[othp].b){
                        target.push_back(other[othp]);
                    }
                    orip++;
                    othp++;
                }

            }
        }
    }

    // rules
    void setVar(DInt var); // update all clauses with a var and _used and _value.
    bool decide(); // decide a unaffected var : return false on decision, true if finished (SAT).
    // fix the value this var as non-decided and give an deletable reason.
    void unit(DInt var, std::vector<DInt>& decCl);
    // fix the value this var as non-decided and give an non-deletable reason.
    void unit(DInt var, int clause);
    void conflict(int clause); // resolve conflict on clause, do all resolution steps.
    void handle(); // take care of the next element in _toUpdate, fail badly if _toUpdate is empty.

    // Check class invariant
    void checkInvariant();

    // Check if a vector of literals (a clause) is indeed a set (unicity + sorted).
    static bool isSet(const std::vector<DInt>& v){
        std::set<DInt> v2(v.begin(), v.end());
        std::vector<DInt> res(v2.begin(),v2.end());
        return v == res;
    }

    // Check if a value is indeed in the set.
    static bool in(DInt di, const std::vector<DInt>& v){
        auto it = std::lower_bound(v.begin(),v.end(),di);
        if(it == v.end()) return false;
        return di == *it;
    }

    // If di is in v then it returns its index else it return UB.
    static int index(DInt di, const std::vector<DInt>& v){
        auto it = std::lower_bound(v.begin(),v.end(),di);
        if(it == v.end()) return -1;
        return it - v.begin();
    }

    // pretty-printing
    friend std::ostream& operator<<(std::ostream& out, const SatSolver::DInt& var);
    friend std::ostream& operator<<(std::ostream& out, const SatSolver::MLit& var);
    friend std::ostream& operator<<(std::ostream& out, const SatSolver::Clause& cl);
public :
    SatSolver(int numVar, bool verbose);

    // Import SatCnf into the solver.
    void import(const SatCnf& sc);

    //Solve a sat Cnf, returns empty vector if UNSAT.
    std::vector<bool> solve();

    // Add a SMT Conflict clause.
    void addSMTConflict(SatCnf::Clause& cl);
};

inline std::ostream& operator<<(std::ostream& out, const SatSolver::DInt& var){
    out << (var.b ? "¬" : "") << var.i +1;
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const SatSolver::MLit& var){
    out << var.var;
    if(&var.decidingCl != nullptr) out << var.decidingCl;
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const SatSolver::Clause& cl){
    for(size_t i = 0 ; i < cl.clause.size() -1 ; ++i){
        out << cl.clause[i] << " v ";
    }
    out << cl.clause.back() << " <- " << cl.wl1 << " <- " << cl.wl2;
    return out;
}



#endif
