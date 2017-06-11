#include "SatSolver.h"

using namespace std;

void SatSolver::checkInvariant(){
#ifndef NDEBUG
    cout << "check";
    // sizes
    if(_model.size() > _numVar){
        cout << "model too big :" << _model.size() << " " <<  _numVar <<  endl;
        printModel();
        cout << endl;
        assert(false);
    }
    assert(_used.size() == _numVar);
    assert(_value.size() == _numVar);
    assert(_watched.size() == 2 * _numVar);

    // model used & value self coherence
    Bitset used(_numVar);
    Bitset value(_numVar);
    Bitset unicity(_numVar);
    used.clear();
    value.clear();
    for(auto& mlit : _model){
        unicity.clear();
        used[mlit.var.i] = true;
        value[mlit.var.i] = ! mlit.var.b;
        assert(isSorted(mlit.decidingCl));
        for(DInt di : mlit.decidingCl){
            assert(!unicity[di.i]);
            unicity[di.i] = true;
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
    if(_used != used){
        cout << "new " << used << endl;
        cout << "old " << _used << endl;
        assert(false);
    }
    for(size_t i= 0 ; i < _numVar ; ++i){
        if(used[i]){
            _value[i] = value[i];
        }
    }

    // TODO _clauses wl check.
    // TODO _watched to wl check.
#endif
}
void SatSolver::setVar(DInt var){
    //cout << "setting : " << var << endl;
    if(_used[var.i]){
        cerr << "var " << var.i+1 << " is already unordered_set" << endl;
        exit(1);
    }
    for(auto cl : _watched[!var]){
        //cout << "to Update : " << cl;
        _toUpdate.push_back(cl);
    }
    _used[var.i] = true;
    _value[var.i] = !var.b;
}

bool SatSolver::decide(){
    checkInvariant();
    assert(_toUpdate.empty());
    int var = _used.usf();
    //cout << "used : " << _used << "usf" << var << endl;
    if(var == -1) return true; // YEAH : SAT
    assert(!_used[var]);
    setVar(DInt(false,var));
    _model.push_back(MLit{DInt(false,var),{}});
    if(_verbose) {
        cout << endl << "Deciding var " << var+1 << endl << "Model : ";
        printModel();
        cout << endl;
    }
    return false;
}

void SatSolver::unit(DInt var, std::vector<DInt> decCl){
    assert(!_used[var.i]);
    assert(isSorted(decCl));
    setVar(var);
    _model.push_back(MLit{var,move(decCl)});
}

void SatSolver::unit(DInt var, int clause){
    //auto& cl = _clauses.at(clause).clause;
    unit(var,_clauses[clause].clause);
}

SatSolver::SatSolver(int numVar, bool verbose)
    : _numVar(numVar), _verbose(verbose), _used(numVar), _value(numVar){
    _used.clear();
    _value.clear();
    _watched.resize(2*numVar);
}

void SatSolver::conflict(int clause){ // conflict by resolution then backjump
    assert((size_t)clause < _clauses.size());
    checkInvariant();
    _toUpdate.clear();
    vector<DInt> R = _clauses[clause].clause;
    if(_verbose ) cout << "starting resolution with : " << R << endl;
    while(!_model.empty() and !R.empty()){
        MLit& cur = _model.back();
        assert(!in(cur.var,R)); // the variable is not in R.
        if(in(!cur.var,R)){
            if(cur.decidingCl.empty()){ // start backjump
                if(_verbose){
                    cout << "conflict end on var : " << cur.var << " with : " << R << endl;
                    cout << "old model :";
                    printModel();
                    cout << endl;
                }
                int i;
                int lastDeciLit = _model.size() -1;
                for(i = _model.size() -2 ; i >= 0 ; --i){
                    if(in(!_model[i].var,R)) {
                        if(_verbose) cout << "backjump breaked at : " << i
                                          << " cutting at : " << lastDeciLit << endl;
                        break;
                    }
                    if(_model[i].decidingCl.empty()) lastDeciLit = i;
                }
                DInt v = cur.var;
                v = !v;
                if(_verbose) {
                    cout << "before backjump model :" << endl;
                    printModel();
                    cout << endl <<  "and used :" << _used << endl;
                }
                for(size_t i = lastDeciLit ; i < _model.size() ; ++i){
                    _used[_model[i].var.i] = false;
                }
                _model.resize(lastDeciLit);
                unit(v,R);
                checkInvariant();
                return;
            }
            else{
                fusion(R,cur.decidingCl);
                if(_verbose) cout << "updating R on : " << cur.var << " with : " << R << endl;
                _used[_model.back().var.i] = false;
                _model.pop_back();
                if(_verbose) {
                    cout << "new model :" << endl;
                    printModel();
                    cout << endl <<  "and used :" << _used << endl;
                }
            }
        }
        else {
            _used[_model.back().var.i] = false;
            _model.pop_back();
        }
    }
    cout << "-------------------UNSAT----------------------" << endl;
    throw 0; // NOOOOOOOOOO : UNSAT
}

void SatSolver::handle(){
    checkInvariant();
    DInt clNum = _toUpdate.front();
    _toUpdate.pop_front();
    Clause& cl = _clauses.at(clNum.i);
    if(clNum.b){ // mutating wl2
        if(_verbose){
            cout << endl << "Updating wl2 in " << clNum.i << " : " << cl << endl;
            cout << "in model : ";
            printModel();
            cout << endl;
        }
        assert(isFalse(cl.clause[cl.wl2]));
        if(isTrue(cl.clause[cl.wl1])) return;
        for(size_t i = 0 ; i < cl.clause.size() ; ++ i){
            if (i == cl.wl1 or i == cl.wl2) continue;
            if(!isFalse(cl.clause[i])){
                if(_verbose) cout << "new wl found " << i << endl;
                _watched[cl.clause[cl.wl2]].erase(clNum);
                cl.wl2 = i;
                _watched[cl.clause[cl.wl2]].insert(clNum);
                return;
            }
        }
        if(isFalse(cl.clause[cl.wl1])){
            conflict(clNum.i);
            if(_verbose){
                cout << "conflict : ";
                printModel();
                cout << endl;
            }
            return;
        }
        unit(cl.clause[cl.wl1],clNum.i);
        if(_verbose){
            cout << "unit  on : " << cl.clause[cl.wl1] << " : ";
            printModel();
            cout << endl;
        }
    }
    else{ // mutating wl1
        if(_verbose){
            cout << endl << "Updating wl1 in " << clNum.i << " : " << cl << endl;
            cout << "in model : ";
            printModel();
            cout << endl;
        }
        assert(isFalse(cl.clause[cl.wl1]));
        if(isTrue(cl.clause[cl.wl2])) return;
        for(size_t i = 0 ; i < cl.clause.size() ; ++ i){
            if (i == cl.wl1 or i == cl.wl2) continue;
            if(!isFalse(cl.clause[i])){
                if(_verbose) cout << "new wl found " << i << endl;
                _watched[cl.clause[cl.wl1]].erase(clNum);
                cl.wl1 = i;
                _watched[cl.clause[cl.wl1]].insert(clNum);
                return;
            }
        }
        if(isFalse(cl.clause[cl.wl2])){
            conflict(clNum.i);
            if(_verbose){
                cout << "conflict : ";
                printModel();
                cout << endl;
            }
            return;
        }
        unit(cl.clause[cl.wl2],clNum.i);
        if(_verbose){
            cout << "unit  on : " << cl.clause[cl.wl2] << " : ";
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

