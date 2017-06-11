#ifndef SATSOLVER_H
#define SATSOLVER_H

#include <utility>
#include <vector>
//#include <set>
#include <deque>
#include <iostream>
#include "SatCnf.h"
#include "Bitset.h"
#include "prettyprint.hpp"

// this class hold the sat solver state
// the class invariant is programmatically stated in checkInvariant();
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
        DInt(){}
    }; // 2n : var n, 2n+1 neg var n.

    
    struct MLit{
        DInt var;
        // if decision == {} : the var has been decided else this is the deciding clause.
        std::vector<DInt> decidingCl; // sorted array
    };
    size_t _numVar;
    bool _verbose;
    bool firstTime;
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

    static void fusion (std::vector<DInt>& target, const std::vector<DInt>& other){
        //std::cout << "before fusion : " << target << " " << other << std::endl;
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
            //std::cout << "one turn : " << target;
        }
        //std::cout << "after fusion : " << target << " " << other << std::endl;
    }

    // rules
    void setVar(DInt var); // update all clauses with a var and _used and _value.
    bool decide(); // decide a unaffected var : return false on decision, true if finished (SAT).
    void unit(DInt var, std::vector<DInt> decCl); // fix this var as non-decided and give the reason.
    void unit(DInt var, int clause); // fix this var as non-decided and give the reason.
    void conflict(int clause); // resolve conflict on clause, for now only backtrack. TODO resolve
    void handle(); // take care of the next element in _toUpdate, fail badly if to update is empty.

    // check class invariant
    void checkInvariant();
    static bool isSorted(const std::vector<DInt>& v){
        std::set<DInt> v2(v.begin(), v.end());
        std::vector<DInt> res(v2.begin(),v2.end());
        return v == res;
    }

    static bool in(DInt di, const std::vector<DInt>& v){
        auto it = std::lower_bound(v.begin(),v.end(),di);
        if(it == v.end()) return false;
        return di == *it;
    }

    // if di in v return its index else return UB.
    static int index(DInt di, const std::vector<DInt>& v){
        auto it = std::lower_bound(v.begin(),v.end(),di);
        if(it == v.end()) return -1;
        return it - v.begin();
    }

    // pretty-printing
    friend std::ostream& operator<<(std::ostream& out, SatSolver::DInt var);
    friend std::ostream& operator<<(std::ostream& out, SatSolver::MLit var);
    friend std::ostream& operator<<(std::ostream& out, SatSolver::Clause cl);
public :
    SatSolver(int numVar, bool verbose);

    // import SatCnf into the solver.
    void import(const SatCnf& sc);

    //solve a sat Cnf, returns empty vector if UNSAT.
    std::vector<bool> solve();

    // add a SMT Conflict clause.
    void addSMTConflict(SatCnf::Clause& cl);
};

inline std::ostream& operator<<(std::ostream& out, SatSolver::DInt var){
    out << (var.b ? "¬" : "") << var.i +1;
    return out;
}

inline std::ostream& operator<<(std::ostream& out, SatSolver::MLit var){
    out << var.var << var.decidingCl;
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
