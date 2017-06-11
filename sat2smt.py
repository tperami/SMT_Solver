from math import sqrt, ceil
from random import shuffle

nbClause = 0
nbVarSat = 0
nbVarSmt = 0
varToEq = []

def shuffled(l):
    shuffle(l)
    return l

try:
    while True:
        line = input()
        if line[0] == 'c':
            continue
        elif line[0] == 'p':
            sline = line.split()
            assert(sline[1] == "cnf")
            nbVarSat = int(sline[2])
            nbVarSmt = ceil(sqrt(2*nbVarSat))+1
            nbClause = int(sline[3])
            print("p", "cnf", nbVarSmt, nbClause)
            varToEq = shuffled([(j+1, i+1) for i in range(nbVarSmt) for j in range(i)])[:nbVarSat]
            assert(len(varToEq) == nbVarSat)
        elif line[0] == '%':
            break
        else:
            lits = list(map(int, line.split()))
            print(" ".join(map(lambda x: "" if x == 0 else (str(varToEq[abs(x)-1][0]) + ("=" if x > 0 else "<>") + str(varToEq[abs(x)-1][1])), lits)))
except EOFError:
    pass

