#include <iostream>
#include "SmtCnf.h"
#include <fstream>
#include <cerrno>
#include <cstring>


using namespace std;

int main(int argc, char**argv){
    if(argc <=1) {
        cerr << "No input file !" << endl;
        return 1;
    }
    string s = argv[1];
    ifstream file(s);
    istream& in = (s == "-" ? cin : file);
    if(s != "-") cout << "Opening " << s << " : " << endl;
    try{
        in.exceptions(istream::failbit); // immediate launch if file can't be opened
        SmtCnf sc(in);
        cout << "Solving" << endl;
        cout << sc;

        //auto v = solve(sc);
        //if(v.empty()) cout << "UNSAT" << endl;
        //else cout << v;
    }
    catch(std::exception& e){
        if(!errno) cerr << e.what() << endl;
        else cerr << strerror(errno) << endl;
        return 1;
    }
    return 0;
}
