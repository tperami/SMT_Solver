#include "SatSolver.h"

using namespace std;

void SatSolver::setVar(DInt var){
    //cout << "setting : " << var << endl;
    if(_used[var.i]){
        cerr << "var " << var.i+1 << " is already set" << endl;
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
    assert(_toUpdate.empty());
    int var = _used.usf();
    //cout << "used : " << _used << "usf" << var << endl;
    if(var == -1) return true; // YEAH : SAT
    assert(!_used[var]);
    setVar(DInt{false,var});
    _model.push_back(MLit{DInt{false,var},true});
    if(_verbose) {
        cout << endl << "Deciding var " << var+1 << endl << "Model : ";
        printModel();
        cout << endl;
    }
    return false;
}

void SatSolver::unit(DInt var){
    assert(!_used[var.i]);
    setVar(var);
    _model.push_back(MLit{var,false});
}


SatSolver::SatSolver(int numVar, bool verbose)
    : _numVar(numVar), _verbose(verbose), _used(numVar), _value(numVar){
    _used.clear();
    _value.clear();
    _watched.resize(2*numVar);
}

void SatSolver::conflict(int clause){
    _toUpdate.clear();
    while(!_model.empty()){
        _used[_model.back().var.i] = false;
        if(_model.back().decided){
            _model.back().decided = false;
            _model.back().var.b = ! _model.back().var.b;
            setVar(_model.back().var);
            return;
        }
        else{
            _model.pop_back();
        }
    }
    cout << "-------------------UNSAT----------------------" << endl;
    throw 0; // NOOOOOOOOOO : UNSAT
}

void SatSolver::handle(){
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
            conflict(clNum);
            if(_verbose){
                cout << "conflict : ";
                printModel();
                cout << endl;
            }
            return;
        }
        unit(cl.clause[cl.wl1]);
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
            conflict(clNum);
            if(_verbose){
                cout << "conflict : ";
                printModel();
                cout << endl;
            }
            return;
        }
        unit(cl.clause[cl.wl2]);
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
                printWatched();
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

