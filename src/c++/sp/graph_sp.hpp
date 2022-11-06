#ifndef _graph_
#define _graph_
#include <vector>
#include <cassert>
#include <unordered_map>

#define inf 1000000000 // nine 0s

struct edge_sp {
    int start;
    int to;
    int length;
    int cost;
};

class GraphSP {
    public:
    int num_nodes = -1;
    int num_edges;
    std::vector<std::unordered_map<int, edge_sp> > edges;
    std::vector<int> degree;

    public:
    void init() {
        int n = num_nodes;
        assert(n != -1);
        edges.resize(n+5);
        degree.resize(n+5, 0);
    }
    
    void init_degree () {
       
        for (int i=0; i<num_nodes; i++)
        {
            for (auto item : edges[i])
            {
                int u = i; // source
                int v = item.first; // sink
                if (edges[v].find(u) == edges[v].end())
                {
                    edge_sp e_vu {v, u, inf, 0}; // add INF edge
                    edges[v].insert( std::make_pair(u, e_vu) );
                    this->num_edges++;

                }

            }
        }

        for (int i=0; i<num_nodes; i++ ) // amendment the degree value
        {
            degree[i] = edges[i].size();
        }




    }
};

#endif