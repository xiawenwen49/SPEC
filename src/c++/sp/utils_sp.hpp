#ifndef _h2h_utils_
#define _h2h_utils_

#include <iostream>
#include <vector>
#include <sys/stat.h>

#include "graph_sp.hpp"

using namespace std;

inline bool exists (const std::string& filename) {
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}

void read_graph_sp(const char *file_name, GraphSP *g, int graph_type, bool d_c_inverse) {
    int num_of_vertex, num_of_edge;
    FILE *topo_f = fopen(file_name, "r");
    if (graph_type == 1) { // node index also must starts from 0.
        int w, c, u, v = 0;
        int res = fscanf(topo_f, "%d %d", &num_of_vertex, &num_of_edge);
        
        g->num_nodes = num_of_vertex;
        g->num_edges = num_of_edge;
        g->init();

        for (; fscanf(topo_f, "%d %d %d %d", &u, &v, &w, &c) == 4;) {
            if (d_c_inverse) { swap(w, c); }

            edge_sp e_uv {u, v, w, c};
            edge_sp e_vu {v, u, w, c};
            if (g->edges[u].find(v) == g->edges[u].end()) 
            { 
                g->edges[u].insert(make_pair(v, e_uv)); // as directed
                // g->edges[v].insert(make_pair(u, e_vu)); // as undirected
            }


        }

    }
    else if (graph_type == 2) {
        int res = fscanf(topo_f, "%d", &num_of_vertex); // node index starts from 0.
        
        g->num_nodes = num_of_vertex;
        g->init();
        int num_edges = 0;
        int w, c, u, v = 0;
        for (; fscanf(topo_f, "%d:", &u) == 1;) {
        for (; fscanf(topo_f, "%d", &v) == 1;) {
            if (v == -1) break;
            int ret_val = fscanf(topo_f, "%d %d", &w, &c);
            if (ret_val < 0) {
                std::cout << "input file error" << std::endl;
                exit(0);
            }
            if (d_c_inverse) { swap(w, c); } // important

            edge_sp e_uv {u, v, w, c};
            edge_sp e_vu {v, u, w, c};
            if (g->edges[u].find(v) == g->edges[u].end()) 
            { 
                g->edges[u].insert(make_pair(v, e_uv)); // as directed
                num_edges++;
            }
        }

        }
        g->num_edges = num_edges;
    }
    g->init_degree();
    
    fclose(topo_f);
}

class Task {
    public:
    int ntask;
    vector<int> source_l_;
    vector<int> sink_l_;
    vector<int> cost_l_;

    public:
    Task() {
        ;
    }
    Task(int ntask) :
        ntask(ntask) {
        ;
    }
    void load_query(const char *file_name) {
        FILE *topo_f;
        topo_f = fopen(file_name, "r");

        int u, v, cost_lower, cost_upper, cost_limit;
        for (; fscanf(topo_f, "%d %d %d %d %d\n", &u, &v, &cost_lower, &cost_upper, &cost_limit) == 5;) {
            source_l_.push_back(u);
            sink_l_.push_back(v);
            cost_l_.push_back(cost_limit);
        }
        fclose(topo_f);
        ntask = source_l_.size();
        }
};

#endif