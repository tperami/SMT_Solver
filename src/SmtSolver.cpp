#include "SmtSolver.h"
#include <queue>
#include <cassert>
#include <iostream>
#include "SatSolver.h"

std::pair<int, int> mkOrderedPair(int a, int b) {
    if(a <= b) {
        return std::make_pair(a, b);
    } else {
        return std::make_pair(b, a);
    }
}

std::pair<SmtSatKernel, SatCnf> gene(const SmtCnf& sc) {
    SmtSatKernel res;

    SatCnf resCnf(-1);

    for(const SmtCnf::Clause& c : sc.clauses) {
        SatCnf::Clause curClause;
        for(const SmtCnf::Literal& l : c.literals) {
            int v1 = l.var1;
            int v2 = l.var2;
            if(v2 < v1) {
                std::swap(v1, v2);
            }
            //v1 <= v2
            std::pair<int, int> pair = std::make_pair(v1, v2);
            if(res.to.count(pair)) {
            } else {
                res.to.insert(std::make_pair(pair, res.from.size()));
                res.from.push_back(pair);
            }
            SatCnf::Literal curLit;
            curLit.var = res.to[pair];
            curLit.neg = (l.type == SmtCnf::Literal::NOTEQ);
            curClause.literals.push_back(curLit);
        }
        resCnf.clauses.push_back(curClause);
    }

    resCnf._numVar = res.from.size();

    return std::make_pair(res, resCnf);
}

std::pair<bool, std::vector<int>> decide(const SmtSatKernel& ker, std::vector<bool> vals) {
    assert(ker.from.size() == vals.size());
    std::vector<std::vector<int>> graph(vals.size());
    std::vector<std::pair<int, int>> forbidden;

    //On génère le graphe d'égalité et quelles sont les égalités impossibles (forbidden)
    for(size_t i = 0; i < ker.from.size(); ++i) {
        int v1 = ker.from[i].first;
        int v2 = ker.from[i].second;
        if(vals[i]) {
            graph[v1].push_back(v2);
            graph[v2].push_back(v1);
        } else {
            forbidden.push_back(std::make_pair(v1, v2));
        }
    }

    //Algorithme pour trouver les composantes connexes (linéaire)
    struct CompConn {
        const std::vector<std::vector<int>>& graph;
        std::vector<int> comps;
        CompConn(const std::vector<std::vector<int>>& _graph)
            : graph(_graph), comps(_graph.size(), -1) {}

        void dfs(size_t node, int comp) {
            if(comps[node] != -1) return;
            comps[node] = comp;
            for(int neigh : graph[node]) {
                dfs(neigh, comp);
            }
        }

        void compute() {
            int curComp = 0;
            for(size_t i = 0; i < graph.size(); ++i) {
                if(comps[i] == -1) {
                    dfs(i, curComp);
                    curComp += 1;
                }
            }
        }
    };

    CompConn cc(graph);
    cc.compute();

    //Calcul des "forbidden" qui crééent des conflits
    std::vector<size_t> badForbidden;
    for(size_t i = 0; i < forbidden.size(); ++i) {
        if(cc.comps[forbidden[i].first] == cc.comps[forbidden[i].second]) {
            badForbidden.push_back(i);
        }
    }

    if(badForbidden.empty()) {
        return std::make_pair(true, cc.comps);
    }

    //BFS pour calculer le plus court chemin d'un point à tous les autres
    struct ShortestPath {
        const std::vector<std::vector<int>>& graph;
        const SmtSatKernel& ker;
        std::vector<int> dists;
        int startNode;
        ShortestPath(const std::vector<std::vector<int>>& _graph, const SmtSatKernel& _ker)
            : graph(_graph), ker(_ker), dists(_graph.size(), -1), startNode(-1) {}

        void compute(int start) {
            startNode = start;
            dists.assign(graph.size(), -1);
            dists[start] = 0;
            std::queue<int> q;
            q.push(start);
            while(!q.empty()) {
                int v = q.front();
                q.pop();
                for(int neigh : graph[v]) {
                    if(dists[neigh] == -1) {
                        dists[neigh] = dists[v]+1;
                        q.push(neigh);
                    }
                }
            }
        }

        //Donne la "preuve" de l'égalité entre startNode et to
        std::vector<int> recoverCounterExample(int to) {
            std::vector<int> res;
            int cur = to;
            while(cur != startNode) {
                for(int neigh : graph[cur]) {
                    if(dists[cur] == dists[neigh]+1) {
                        res.push_back(ker.to.at(mkOrderedPair(cur, neigh)));
                        cur = neigh;
                        break;
                    }
                }
            }
            res.push_back(ker.to.at(mkOrderedPair(startNode, to)));
            return res;
        }
    };

#if 0
    //Algorithme linéaire mais ne fournit pas le contre-exemple minimal
    //On prends un "forbidden" qui créé conflit, et on trouve le contre-exemple
    //minimal pour celui-ci

    auto pair = forbidden[badForbidden.front()];
    int v1 = pair.first;
    int v2 = pair.second;

    ShortestPath sp(graph, ker);
    sp.compute(v1);
    return std::make_pair(false, sp.recoverCounterExample(v2));
#else
    //Algorithme quadratique mais fournit le contre-exemple minimal
    //À chaque tour de boucle, on prends le sommet qui créé le plus de conflits-non-traités, on calcule
    //les distances à partir de lui et ainsi on prends le "chemin minimal" entre tous
    //les conflits, qui est le contre-exemple minimal

    ShortestPath sp(graph, ker);
    int bestDist = 1<<30;
    std::vector<int> bestCounterExample;

    //nombre de foit qu'une variable apparait dans un conflit
    std::vector<int> nbForb(vals.size(), 0);
    //liste d'adjacence pour les conflits
    std::vector<std::vector<int>> graphForb(vals.size(), std::vector<int>());
    for(const auto& pair : forbidden) {
        int v1 = pair.first;
        int v2 = pair.second;
        nbForb[v1] += 1;
        nbForb[v2] += 1;
        graphForb[v1].push_back(v2);
        graphForb[v2].push_back(v1);
    }

    while(true) {
        //indice de la variable qui créé le plus de conflits
        size_t maxForb = 0;
        for(size_t i = 1; i < nbForb.size(); ++i) {
            if(nbForb[i] > nbForb[maxForb]) {
                maxForb = i;
            }
        }
        if(nbForb[maxForb] == 0) break;
        nbForb[maxForb] = 0;

        sp.compute(maxForb);
        int bestNeigh = -1;
        int dist = 1<<30;
        for(int neigh : graphForb[maxForb]) {
            if(nbForb[neigh] != 0) {
                nbForb[neigh] -= 1;
                if(dist > sp.dists[neigh]) {
                    bestNeigh = neigh;
                    dist = sp.dists[neigh];
                }
            }
        }
        if(dist < bestDist) {
            bestDist = dist;
            bestCounterExample = sp.recoverCounterExample(bestNeigh);
        }
    }

    return std::make_pair(false, bestCounterExample);
#endif
}

std::vector<int> solve(const SmtCnf& sc, bool smtVerbose, bool satVerbose) {
    auto pair_ = gene(sc);
    SmtSatKernel ker = pair_.first;
    SatCnf satc = pair_.second;
    SatSolver ss(satc._numVar, satVerbose);
    ss.import(satc);
    while(true) {
        std::vector<bool> vals = ss.solve();
        if(vals.empty()) {
            return std::vector<int>();
        }
        auto pair = decide(ker, vals);
        if(pair.first) {
            return pair.second;
        }
        SatCnf::Clause newClause;
//Cette condition préprocesseur est là pour tester la performance sans l'algorithme
//qui trouve un contre-exemple petit
#if 1
        for(int i : pair.second) {
#else
        for(int i = 0; i < vals.size(); ++i) {
#endif
            newClause.literals.push_back(SatCnf::Literal{vals[i], i});
        }
        if(smtVerbose) {
            std::cout << "Valuation: " << vals << std::endl;
            std::cout << "Counter example: " << pair.second << std::endl;
            std::cout << "Adding clause " << newClause << std::endl;
        }
        ss.addSMTConflict(newClause);
    }
}

