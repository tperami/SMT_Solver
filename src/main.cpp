#include <iostream>
#include "SmtCnf.h"
#include "SatCnf.h"
#include "SatSolver.h"
#include "SmtSolver.h"
#include <fstream>
#include <cerrno>
#include <cstring>
#include "prettyprint.hpp"


using namespace std;

int main(int argc, char**argv){
    bool satverbose = false;
    bool smtverbose = false;
    try{
        for(int cur  = 1 ; cur < argc ; ++cur){
            string s = argv[cur];
            if(s == "-v"){
                smtverbose = true;
                cout << "Verbose mode activated" << endl;
                continue;
            }
            else if(s == "-vv"){
                satverbose = true;
                smtverbose = true;
                cout << "VeryVerbose mode activated" << endl;
                continue;
            }
            else if(s == "-sat"){
                ++cur;
                if(cur >= argc){
                    cerr << "Not enough argument" <<endl;
                    return 1;
                }
                string filename = argv[cur];
                ifstream file(filename);
                istream& in = (filename == "-" ? cin : file);
                if(s != "-") cout << "Opening " << filename << " : " << endl;
                in.exceptions(istream::failbit); // immediate launch if file can't be opened
                SatCnf sc(in);
                cout << "Solving :" << endl;
                cout << sc << endl;
                SatSolver sats(sc._numVar,satverbose);
                sats.import(sc);
                auto sol = sats.solve();
                cout << "Solution : " << sol << endl;

                if(!sol.empty()){
                    cout << sc.eval(sol) << endl;
                }
                return 0;
            }
            else if(s == "-smt"){
                ++cur;
                if(cur >= argc){
                    cerr << "Not enough argument" <<endl;
                    return 1;
                }
                string filename = argv[cur];
                ifstream file(filename);
                istream& in = (filename == "-" ? cin : file);
                if(s != "-") cout << "Opening " << filename << " : " << endl;
                in.exceptions(istream::failbit); // immediate launch if file can't be opened
                SmtCnf sc(in);
                cout << "Solving" << endl;
                cout << sc;
                auto sol = solve(sc, smtverbose, satverbose);
                cout << "Solution : " << sol << endl;
                if(!sol.empty()) {
                    cout << sc.eval(sol) << endl;
                }

                exit(0);
            }
            /*else if(s == "-gsat"){
                
              }*/
            else{
                cout << "unknown option " << s << endl;
                return 1;
            }
        }
    }
    catch(std::exception& e){
        if(!errno) cerr << e.what() << endl;
        else cerr << strerror(errno) << endl;
        return 1;
    }
    return 0;
}
