#include "SatSolver.h"

using namespace std;

void SatSolver::checkInvariant(){
#ifndef NDEBUG
    // check sizes.
    if(_model.size() > _numVar){
        cout << "model too big :" << _model.size() << " " <<  _numVar <<  endl;
        printModel();
        cout << endl;
        assert(false);
    }
    assert(_used.size() == _numVar);
    assert(_value.size() == _numVar);
    assert(_watched.size() == 2 * _numVar);

    // model & used & value self coherence
    // I'll set used and value to their correct values and then compare them.
    Bitset used(_numVar);
    Bitset value(_numVar);
    used.clear();
    value.clear();
    for(auto& mlit : _model){
        // set used and value
        used[mlit.var.i] = true;
        value[mlit.var.i] = ! mlit.var.b;
        // If decision literal stop here
        if(&mlit.decidingCl == nullptr) continue;
        // The deciding clause must be a set
        assert(isSet(mlit.decidingCl));

        for(DInt di : mlit.decidingCl){
            // The deciding clause must contained either the current literal
            // or the negation of preceding literal in the model.
            if(di.i == mlit.var.i){
                assert(di.b == mlit.var.b);
            }
            else{
                assert(used[di.i]);
                if(value[di.i] != di.b){
                    cerr << value << " " << di << endl;
                    assert(false);
                }
            }
        }
    }
    assert(_used == used);
    for(size_t i= 0 ; i < _numVar ; ++i){
        if(used[i]){
            assert(_value[i] == value[i]);
        }
    }

#endif
}
void SatSolver::setVar(DInt var){
    // update used and value and add affected clauses to _toUpdate.
    assert(!_used[var.i]);
    for(DInt cl : _watched[!var]){
        _toUpdate.push_back(cl);
    }
    _used[var.i] = true;
    _value[var.i] = !var.b;
}

bool SatSolver::decide(){
    checkInvariant();
    // we can't decide if their is still clauses to be updated.
    assert(_toUpdate.empty());

    // first unaffected var.
    int var = _used.usf();

    // their is no unaffected vars :
    if(var == -1) return true; // YEAH : SAT

    assert(!_used[var]);
    setVar(DInt(false,var));
    _model.push_back(MLit(DInt(false,var),nullptr));
    if(_verbose) {
        cout << endl << "Deciding var " << var+1 << endl << "New model : ";
        printModel();
        cout << endl;
    }
    return false;
}

void SatSolver::unit(DInt var, std::vector<DInt>& decCl){
    assert(!_used[var.i]);
    assert(isSet(decCl));
    setVar(var);
    _model.push_back(MLit(var,&decCl));
}

void SatSolver::unit(DInt var, int clause){
    assert(!_used[var.i]);
    setVar(var);
    _model.push_back(MLit(var,_clauses[clause].clause));
}

SatSolver::SatSolver(int numVar, bool verbose)
    : _numVar(numVar), _verbose(verbose), _used(numVar), _value(numVar){
    _used.clear();
    _value.clear();
    _watched.resize(2*numVar);
}

void SatSolver::conflict(int clause){ // Conflict by resolution then backjump
    assert((size_t)clause < _clauses.size());
    checkInvariant();

    // other clauses to be updated are useless when there is a conflict.
    _toUpdate.clear();
    // new dynamic clauses on heap.
    vector<DInt>& R = *new std::vector<DInt>(_clauses[clause].clause);
    if(_verbose ) cout << endl <<endl << "Conflict on clause : " << R
                       << ". Starting resolution !" << endl;

    // We are going through the model backward until a decision variable in R is met.
    // Then we backjump as far a possible and we add the variable with the clause R.
    while(!_model.empty() and !R.empty()){
        MLit& cur = _model.back();
        assert(!in(cur.var,R)); // the variable is not in R (R should always be a conflict).
        if(in(!cur.var,R)){ // If we are concerned by R.
            if(&cur.decidingCl == nullptr){ // start backjump
                if(_verbose){
                    cout << endl << "Conflict end on decision literal : " << cur.var
                         << " with clause : " << R << endl;
                }
                int i;
                int lastDeciLit = _model.size() -1;
                for(i = _model.size() -2 ; i >= 0 ; --i){
                    if(in(!_model[i].var,R)) {
                        // If we found another variable in the model, we can't backjump past it.
                        break;
                    }
                    if(&_model[i].decidingCl == nullptr) lastDeciLit = i;
                }
                DInt v = cur.var;
                v = !v;
                for(size_t i = lastDeciLit ; i < _model.size() ; ++i){
                    _used[_model[i].var.i] = false;
                }
                _model.resize(lastDeciLit);
                unit(v,R);

                if (_verbose){
                    cout << "New model : ";
                    printModel();
                    cout << endl << "End of Conflict : Return to exploration !" << endl << endl;
                }

                checkInvariant();
                return;
            }
            else{
                fusion(R,cur.decidingCl);

                if(_verbose) cout << endl << "Resolve on var : " << cur.var
                                  << " with new R : " << R << endl;

                _used[_model.back().var.i] = false;
                _model.pop_back();

                if(_verbose) {
                    cout << "New model : ";
                    printModel();
                    cout << endl;
                }
            }
        }
        else {// If we are not concerned by R, just pop back the model.
            _used[_model.back().var.i] = false;
            _model.pop_back();
        }
    }
    cout << "-------------------UNSAT----------------------" << endl;
    throw 0;
}

void SatSolver::handle(){
    checkInvariant();
    DInt clNum = _toUpdate.front();
    _toUpdate.pop_front();
    Clause& cl = _clauses.at(clNum.i);
    if(clNum.b){ // mutating wl2
        if(_verbose){
            cout << endl << "Updating second watched literal because of "
                 << !cl.clause[cl.wl1] << " in clause "
                 << clNum.i << " : " << cl << endl;
            cout << "in the model : ";
            printModel();
            cout << endl;
        }

        assert(isFalse(cl.clause[cl.wl2]));

        // First case : the other watched literal is true.
        if(isTrue(cl.clause[cl.wl1])) return;

        for(size_t i = 0 ; i < cl.clause.size() ; ++ i){
            if (i == cl.wl1 or i == cl.wl2) continue;
            if(!isFalse(cl.clause[i])){
                // Second case, we can still watch another literal
                if(_verbose) cout << "new watched literal found " << cl.clause[i]
                                  << " at : " << i << endl;

                _watched[cl.clause[cl.wl2]].erase(clNum);
                cl.wl2 = i;
                _watched[cl.clause[cl.wl2]].insert(clNum);
                return;
            }
        }

        // third case we can't find other places and the other WL is false : conflict.
        if(isFalse(cl.clause[cl.wl1])){
            conflict(clNum.i);
            return;
        }
        // last case, the only not false literal is the other one.
        unit(cl.clause[cl.wl1],clNum.i);
        if(_verbose){
            cout << "Applied unit on var : " << cl.clause[cl.wl1] << endl;
            cout << "New model : ";
            printModel();
            cout << endl;
        }
    }
    else{ // mutating wl1
        if(_verbose){
            cout << endl << "Updating first watched literal because of "
                 << !cl.clause[cl.wl1] << " in clause "
                 << clNum.i << " : " << cl << endl;
            cout << "in the model : ";
            printModel();
            cout << endl;
        }
        assert(isFalse(cl.clause[cl.wl1]));
        if(isTrue(cl.clause[cl.wl2])) return;
        for(size_t i = 0 ; i < cl.clause.size() ; ++ i){
            if (i == cl.wl1 or i == cl.wl2) continue;
            if(!isFalse(cl.clause[i])){
                if(_verbose) cout << "New watched literal found " << cl.clause[i]
                                  << " at : " << i << endl;
                _watched[cl.clause[cl.wl1]].erase(clNum);
                cl.wl1 = i;
                _watched[cl.clause[cl.wl1]].insert(clNum);
                return;
            }
        }
        if(isFalse(cl.clause[cl.wl2])){
            conflict(clNum.i);
            return;
        }
        unit(cl.clause[cl.wl2],clNum.i);
        if(_verbose){
            cout << "Applied unit on var : " << cl.clause[cl.wl2] << endl;
            cout << "New model : ";
            printModel();
            cout << endl;
        }
    }
}

void SatSolver::import(const SatCnf& sc){
    assert(sc._numVar == _numVar);
    for(auto cl : sc.clauses){
        addSMTConflict(cl);
    }
}

void SatSolver::addSMTConflict(SatCnf::Clause& cl){
    auto toDInt = [](SatCnf::Literal lit){ return DInt{lit.neg,lit.var};};
    bool begin = _model.empty();
    if(cl.literals.size() == 0) return; // this clause is satisfiable
    else{
        Clause cl2;
        for(auto lit : cl.literals){
            cl2.clause.push_back(toDInt(lit));
        }
        sort(cl2.clause.begin(), cl2.clause.end());
        if(begin){
            cl2.wl1 = 0;
            cl2.wl2 = cl.literals.size() -1;
        }
        else{
            bool second = false;
            for(int i = _model.size() -1 ; i >= 0 ; -- i){
                if(in(!_model[i].var,cl2.clause)){
                    if(!second){
                        cl2.wl1 = index(!_model[i].var,cl2.clause);
                        second = true;
                    }
                    else{
                        cl2.wl2 = index(!_model[i].var,cl2.clause);
                        break;
                    }
                }
            }
        }
        _watched[cl2.clause[cl2.wl1]].insert(DInt(false,_clauses.size()));
        _watched[cl2.clause[cl2.wl2]].insert(DInt(true,_clauses.size()));
        if(_verbose){
            cout << "Creating clause " << _clauses.size() << " : " << cl2 << endl;
            //printWatched();
        }
        _clauses.push_back(cl2);
    }
}
std::vector<bool> SatSolver::solve(){
    try{
        if(!_model.empty()){
            conflict(_clauses.size() -1);
            goto middle;
        }
        while(!decide()){
        middle:
            while(!_toUpdate.empty()){
                handle();
            }
        }
    }
    catch(int i){
        return {};
    }

    if(_verbose){
        cout << "SAT with model : ";
        printModel();
        cout << endl;
    }

    vector<bool> res;
    for(size_t i = 0 ; i <_numVar ; ++i){
        res.push_back(_value[i]);
    }
    return res;
}

