#include "SatSolver.h"

using namespace std;

void SatSolver::checkInvariant(){
#ifndef NDEBUG
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
    used.clear();
    value.clear();
    for(auto& mlit : _model){
        used[mlit.var.i] = true;
        value[mlit.var.i] = ! mlit.var.b;
        for(DInt di : mlit.decidingCl){
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

void SatSolver::unit(DInt var, std::set<DInt> decCl){
    assert(!_used[var.i]);
    setVar(var);
    _model.push_back(MLit{var,move(decCl)});
}

void SatSolver::unit(DInt var, int clause){
    auto& cl = _clauses.at(clause).clause;
    unit(var,std::set<DInt>(cl.begin(),cl.end()));
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
    auto& cl = _clauses[clause].clause;
    set<DInt> R(cl.begin(), cl.end());
    if(_verbose ) cout << "starting resolution with : " << R << endl;
#ifndef NDEBUG
    auto test = R;
    for(auto& c : _model){
        test.erase(!c.var);
    }
    if(!test.empty()){
        cout << "FAIL : " << test << endl;
        assert(false);
    }
#endif
    while(!_model.empty() and !R.empty()){
        MLit& cur = _model.back();
        assert(R.count(cur.var) == 0); // the variable is not in R.
        if(R.count(!cur.var)){
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
                    if(R.count(!_model[i].var)) {
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
                if(_verbose) {
                    cout << "after backjump model :" << endl;
                    printModel();
                    cout << endl <<  "and used :" << _used << endl;
                }
                //_model.pop_back();
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
            if(_verbose) {
                cout << "unsetting " << _model.size() << endl;
                cout << "old model :" << endl;
                printModel();
                cout << endl <<  "and used :" << _used << endl;
            }
            _used[_model.back().var.i] = false;
            _model.pop_back();
            if(_verbose) {
                cout << "new model :" << endl;
                printModel();
                cout << endl <<  "and used :" << _used << endl;
            }
        }
        /*if(_model.back().decidingCl.empty()){ // decision literal.
            DInt v = _model.back().var;
            v = !v;
            _model.pop_back();
            unit(v,clause);
            return;
        }
        else{
            _model.pop_back();
            }*/
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

std::vector<bool> SatSolver::solve(const SatCnf& sc){
    assert(sc._numVar == _numVar);
    auto toDInt = [](SatCnf::Literal lit){ return DInt{lit.neg,lit.var};};
    size_t clNum = 0;
    for(auto cl : sc.clauses){
        if(cl.literals.size() == 0) continue; // this clause is satisfiable
        else{
            Clause cl2;
            for(auto lit : cl.literals){
                cl2.clause.push_back(toDInt(lit));
            }
            cl2.wl1 = 0;
            _watched[cl2.clause[cl2.wl1]].insert(DInt(false,clNum));
            cl2.wl2 = cl.literals.size() -1;
            _watched[cl2.clause[cl2.wl2]].insert(DInt(true,clNum));
            if(_verbose){
                cout << "Creating clause " << clNum << " : " << cl2 << endl;
                //printWatched();
            }
            ++ clNum;
            _clauses.push_back(cl2);
        }
    }

    // finished loading, run solver !
    try{
        while(!decide()){
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

