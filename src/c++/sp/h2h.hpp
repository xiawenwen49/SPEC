
#ifndef _h2h_
#define _h2h_
#include <vector>
#include <tuple>
#include <algorithm>
#include <thread>
#include <cassert>
#include <set>
#include <map>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <iterator>
#include <sstream>

#include "third_libs/indicators/cursor_control.hpp"
#include "third_libs/indicators/progress_bar.hpp"
#include "third_libs/ThreadPool.h"

#include "graph_sp.hpp"

using namespace std;

#define inf 1000000000 // nine 0s




#ifndef _thread_pool_
#define _thread_pool_

#define thread_pool_size 70
#define mtx_num 1000
std::mutex thd_pool_mtx;
std::vector<std::mutex> thd_pool_mtx_list(mtx_num);

ThreadPool thd_pool(thread_pool_size);
#endif


struct DPTreeNode {
    // int main;
    int depth;
    int fa;
    vector<int> nodes; // nodes, phi, phi_cost, order must match
    vector<int> phi;
    vector<int> phi_cost; // edge cost

    vector<int> anc;
    vector<int> sons;

    vector<int> position;
    vector<int> distance;
    vector<int> index_cost; // index for cost
    
    

};

struct DegNode {
    int node;
    int degree;
};

struct cmp_degree {
	bool operator () (DegNode x,DegNode y) {
		if (x.degree!=y.degree) return x.degree<y.degree;	
		return x.node < y.node;
	}
};

struct QueryRes {
    int distance;
    int cost;
};




class H2H {
    public:
    GraphSP * g;
    GraphSP * g_h;
    int n; // number of nodes

    vector<DPTreeNode> X;
    int X_root;
    vector<int> pi; // node -> pi_value
    vector<int> pi_inv; // pi_value -> node
    // vector<int> degree; // initial node degree
    // vector<int> depthes;

    public:
    void init(GraphSP *g);
    void DP_tree_decomposition(int n);
    vector<DegNode> eliminate_v(GraphSP *g_h, int node);
    vector<DegNode> eliminate_v_multi_thread(GraphSP *g_h, int node);
    void build_h2h_index();
    void update_degree_set(set<DegNode, cmp_degree> &degree_set, GraphSP *g_h, vector<DegNode> influenced_nodes);
    void dfs_set_depth(int root, int dep);
    void sort_X_nodes();
    int lca(int x, int y);
    QueryRes query(int x, int y);
    void save_index(const char * index_file);
    void load_index(const char * index_file);
    void load_index_multi_thread(const char * index_file);
    // void load_index_worker(int start, int end, vector<string> *all_lines);

};


void H2H::init(GraphSP *g_input) {
    // build index
    g = g_input;
    g_h = new GraphSP(*g_input);
    n = g->num_nodes;

    X.resize(n+5);
    pi.resize(n+5);
    pi_inv.resize(n+5);
}


void H2H::DP_tree_decomposition(int n) {
    indicators::ProgressBar bar{
        indicators::option::PrefixText{"Build tree"},
    };
    indicators::ProgressBar bar_parent{
        indicators::option::PrefixText{"Set parent"},
    };
    
    
    indicators::show_console_cursor(false);

    set<DegNode, cmp_degree> degree_set;
    degree_set.clear();
    for (int i=0; i<n; i++) { // node index: 0 --- n-1
        degree_set.insert(DegNode{i, g_h->degree[i]} );
    }
    
    for (int order=0; order<n; order++) {
        int node = (*degree_set.begin()).node;
        pi[node] = order;
        pi_inv[order] = node;

        for ( auto key_edge : g_h->edges[node] ) {
            int neig = key_edge.second.to;
            int phi = key_edge.second.length;
            int cost = key_edge.second.cost;
            
            X[node].nodes.push_back(neig);
            X[node].phi.push_back(phi);
            X[node].phi_cost.push_back(cost);
        }
        X[node].nodes.push_back(node);
        X[node].phi.push_back(0); // edge's distance
        X[node].phi_cost.push_back(0); // edge's cost

        vector<DegNode> influenced_nodes = eliminate_v_multi_thread(g_h, node);
        update_degree_set(degree_set, g_h, influenced_nodes);

        bar.set_progress( (order+1)*100/n );

    }

    // set parents
    // std::cout << "Set parents" << std::endl;
    for (int u=0; u<n; u++) {
        int fa;
        int fa_order = inf;
        DPTreeNode Tnode = X[u];
        if (Tnode.nodes.size() <= 1) { // the last order node should be the root
            X[u].fa = -1;
            continue;
        }

        for (int neig : Tnode.nodes ) {
            if ( neig != u && pi[neig] < fa_order ) {
                fa_order = pi[neig];
                fa = neig;
            }
        }
        X[u].fa = fa;
        X[fa].sons.push_back(u);

        bar_parent.set_progress( (u+1)*100/n );
    }

    // find root
    int root = 0; // can be any value in 0 -- n-1
    while (X[root].fa != -1) {
        root = X[root].fa;
    }
    X_root = root;

    // set depth
    std::cout << "Set depth" << std::endl;
    dfs_set_depth(X_root, 0);

    // sort nodes
    sort_X_nodes();

    indicators::show_console_cursor(true);
}

void H2H::dfs_set_depth(int root, int dep) {
    X[root].depth = dep;
    for (int i=0; i<X[root].sons.size(); i++) {
        int s = X[root].sons[i];
        dfs_set_depth(s, dep+1);
    }
}

int H2H::lca(int x, int y) {
    while (x != y)
    {   
        if (X[x].depth > X[y].depth ) {
            x = X[x].fa;
        }
        else {
            y = X[y].fa;
        }
    }
    
    return x;
}

QueryRes H2H::query(int x, int y) {
    // std::cout << "lca" << std::endl;
    int l = lca(x, y);
    int dis = inf;
    int cost = inf;

    for (int i=0; i<X[l].nodes.size(); i++) {
        // int s = X[l].position[i];
        int s = X[l].position[i]; // 
        if (dis > X[x].distance[s] + X[y].distance[s]) {
            dis = X[x].distance[s] + X[y].distance[s];
            cost = X[x].index_cost[s] + X[y].index_cost[s];
        }
        else if ( dis == X[x].distance[s] + X[y].distance[s] && cost > X[x].index_cost[s] + X[y].index_cost[s]) { // forgeted before --> a bug
            dis = X[x].distance[s] + X[y].distance[s];
            cost = X[x].index_cost[s] + X[y].index_cost[s];
        }
    }
    return QueryRes{dis, cost};
}

void H2H::save_index (const char * index_file) {
    // depth, fa, nodes, position, distance, index_cost
    indicators::ProgressBar bar{
        indicators::option::PrefixText{"Save index"},
    };
    indicators::show_console_cursor(false);


    ofstream file (index_file);
    if (file.is_open()) {
        for (int i=0; i<n; i++) {
            file << i << "\n";
            file << X[i].depth << "\n";
            file << X[i].fa << "\n";
            for (int j=0; j<X[i].nodes.size(); j++) {
                file << X[i].nodes[j] << " ";
            }
            file << "\n";
            for (int j=0; j<X[i].position.size(); j++) {
                file << X[i].position[j] << " ";
            }
            file << "\n";
            for (int j=0; j<X[i].distance.size(); j++) {
                file << X[i].distance[j] << " ";
            }
            file << "\n";
            for (int j=0; j<X[i].index_cost.size(); j++) {
                file << X[i].index_cost[j] << " ";
            }
            file << "\n";

            file << "\n";

            bar.set_progress( (i+1)*100/n );
        }
        file.close();
        cout << "Index saved\n";
    }
    else cout << "Unable to open file\n";

    indicators::show_console_cursor(true);
}


int load_index_worker(vector<DPTreeNode> *X, int start, int end, vector<string> *all_lines) {
    // start: node index start. 
    // end: node index end
    indicators::ProgressBar bar{
        indicators::option::PrefixText{"Load index"},
    };

    if (start == 0) {    
        indicators::show_console_cursor(false);
    }
    

    int node;
    int depth;
    int fa;
    int line_idx;

    vector<int> values;
    string line;
    std::istringstream iss;


    for (int node=start; node<end; node++ ) {
        line_idx = node * 8;

        node = std::stoi( (*all_lines)[line_idx] );
        depth = std::stoi( (*all_lines)[line_idx+1] );
        fa = std::stoi( (*all_lines)[line_idx+2] );
        (*X)[node].depth = depth;
        (*X)[node].fa = fa;


        iss.str( (*all_lines)[line_idx+3] );
        iss.clear();
        values = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
        (*X)[node].nodes.insert((*X)[node].nodes.end(), values.begin(), values.end() );

        iss.str( (*all_lines)[line_idx+4] );
        iss.clear();
        values = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
        (*X)[node].position.insert((*X)[node].position.end(), values.begin(), values.end() );

        iss.str( (*all_lines)[line_idx+5] );
        iss.clear();
        values = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
        (*X)[node].distance.insert((*X)[node].distance.end(), values.begin(), values.end() );

        iss.str( (*all_lines)[line_idx+6] );
        iss.clear();
        values = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
        (*X)[node].index_cost.insert((*X)[node].index_cost.end(), values.begin(), values.end() );

        if (start ==0) bar.set_progress( (node+1)*100/(end-start) );
    }

    if (start == 0) indicators::show_console_cursor(true);

    return 0;
}

void H2H::load_index_multi_thread(const char * index_file) {
    ifstream file (index_file);
    if (file.is_open()) {
        vector<string> all_lines;
        string str;
        cout << "Read index file into memory" << endl;
        while (getline(file, str))
        {
            all_lines.push_back(str);
        }
        

        int thread_num = thread_pool_size;
        int start;
        int end;
        int interval = n / thread_num;
        vector<future<int> > results;

        cout << "Multi-thread index loading" << endl;
        for (int i=0; i<thread_num-1; i++) {
            start = i*interval;
            end = start + interval;
            results.emplace_back( thd_pool.enqueue(load_index_worker, &X, start, end, &all_lines) );
        }
        start = end;
        end = n;
        results.emplace_back( thd_pool.enqueue(load_index_worker, &X, start, end, &all_lines) );

        for(auto && result: results) {
            result.get();
        }

    }
    else cout << "Unable to open file\n";

}

void H2H::load_index (const char * index_file) {
    indicators::ProgressBar bar{
        indicators::option::PrefixText{"Load index"},
    };
    indicators::show_console_cursor(false);

    ifstream file (index_file);
    if (file.is_open()) {

        int node;
        int depth;
        int fa;
        string line;
        
        vector<int> nodes;
        vector<int> position;
        vector<int> distance;
        vector<int> index_cost;
        std::istringstream iss;

        vector<string> all_lines;
        string str;
        cout << "Read index file" << endl;
        while (getline(file, str))
        {
            all_lines.push_back(str);
        }
        cout << "Read index file ended" << endl;

        for (int i=0; i<n; i++) {
            file >> node;
            file >> depth;
            file >> fa;
            X[node].depth = depth;
            X[node].fa = fa;
            getline(file, line); // the \n after fa
            
            getline(file, line); // nodes
            iss.str(line);
            iss.clear();
            nodes = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
            for (auto x : nodes ) {
                X[node].nodes.push_back(x);
            }
            
            getline(file, line); // position
            iss.str(line);
            iss.clear();
            position = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
            for (auto x : position ) {
                X[node].position.push_back(x);
            }

            getline(file, line); // distance
            iss.str(line);
            iss.clear();
            distance = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
            for (auto x : distance ) {
                X[node].distance.push_back(x);
            }

            getline(file, line); // index cost
            iss.str(line);
            iss.clear();
            index_cost = vector<int>(std::istream_iterator<int>(iss), std::istream_iterator<int>() );
            for (auto x : index_cost ) {
                X[node].index_cost.push_back(x);
            }

            getline(file, line); // empty line

            bar.set_progress( (i+1)*100/n );
        }


    }
    else cout << "Unable to open file\n";

    indicators::show_console_cursor(true);
}


// template<vector<int> &pi>
struct comp_sort_phi {
    vector<int> *pi;
    bool operator () (tuple<int, int, int> x, tuple<int, int, int> y) {
    return (*pi)[std::get<0>(x)] > (*pi)[std::get<0>(y)];
    }
};


void H2H::sort_X_nodes() {

    indicators::ProgressBar bar_sort_nodes{
        indicators::option::PrefixText{"Sort nodes"},
    };


    comp_sort_phi comper;
    comper.pi = &pi;

    for (int u=0; u<n; u++ ) {
        vector<tuple<int, int, int> > temp;
        for (int j=0; j<X[u].nodes.size(); j++) {
            temp.push_back(make_tuple(X[u].nodes[j], X[u].phi[j], X[u].phi_cost[j]));
        }
        
        std::sort(temp.begin(), temp.end(), comper );

        for (int j=0; j<X[u].nodes.size(); j++) {
            X[u].nodes[j] = std::get<0>( temp[j] );
            X[u].phi[j] = std::get<1>( temp[j] );
            X[u].phi_cost[j] = std::get<2>( temp[j] );
        }

        bar_sort_nodes.set_progress( (u+1)*100/n );
    }
}


void H2H::update_degree_set(set<DegNode, cmp_degree> &degree_set, GraphSP *g_h, vector<DegNode> influenced_nodes) {
    degree_set.erase(influenced_nodes[0]);
    for (int i=1; i<influenced_nodes.size(); i++) {
        DegNode deg_node = influenced_nodes[i];
        degree_set.erase(deg_node);
        degree_set.insert(DegNode{deg_node.node, g_h->degree[deg_node.node]});
    }

}

int add_new_edges(GraphSP *g_h, int node, vector<int> *neigs, int start, int end) {

    for (int i=start; i<end; i++) {
        for (int j=0; j<(*neigs).size(); j++) {
            if (j == i) continue;

            int u = (*neigs)[i];
            int v = (*neigs)[j];
            if (g_h->edges[u].find(v) == g_h->edges[u].end()) { // no edge
                int dis = g_h->edges[u][node].length+g_h->edges[node][v].length;
                int cost = g_h->edges[u][node].cost+g_h->edges[node][v].cost;
                dis = min(dis, inf); // avoid overflow
                
                g_h->edges[u].insert(make_pair(v, edge_sp{u, v, dis, cost }));
                g_h->degree[u]++;
            }
            else {
                int dis = g_h->edges[u][node].length + g_h->edges[node][v].length;
                int cost = g_h->edges[u][node].cost + g_h->edges[node][v].cost;

                if (dis < g_h->edges[u][v].length ) { // smaller distance
                    g_h->edges[u][v] = edge_sp{u, v, dis, cost}; // update, can not use insert.
                }
                else if ( dis == g_h->edges[u][v].length &&  cost < g_h->edges[u][v].cost ) {
                    // smaller cost
                    g_h->edges[u][v] = edge_sp{u, v, dis, cost}; 
                    
                }
            }
        }

    }

    return 0;

}

vector<DegNode> H2H::eliminate_v_multi_thread(GraphSP *g_h, int node) {
    vector<DegNode> influenced_nodes;
    vector<int> neigs;
    
    influenced_nodes.push_back(DegNode{node, g_h->degree[node]});
    for (auto key_edge : g_h->edges[node]) {
        int neig = key_edge.first;
        neigs.push_back(neig);
        influenced_nodes.push_back(DegNode{neig, g_h->degree[neig]});
    }

    int thread_num = thread_pool_size;
    vector<future<int> > results;

    // add new edges
    if (neigs.size() > 100 ) {
        // vector<thread> all_threads;
        int start = 0;
        int end;
        int interval = neigs.size() / thread_num;

        for (int i=0; i<thread_num-1; i++) {
            start = i*interval;
            end = start + interval;
            results.emplace_back( thd_pool.enqueue(add_new_edges, g_h, node, &neigs, start, end) );
            
        }
        start = end;
        end = neigs.size();
        results.emplace_back( thd_pool.enqueue(add_new_edges, g_h, node, &neigs, start, end) );

        for(auto && result: results) {
            result.get();
        }

    } else {

        add_new_edges(g_h, node, &neigs, 0, neigs.size() );

    }

    // eliminate node
    for (int neig : neigs ) {
        g_h->edges[neig].erase(node);
        g_h->edges[node].erase(neig);
        g_h->degree[neig]--;
        g_h->degree[node]--;
    }
    return influenced_nodes;
}

vector<DegNode> H2H::eliminate_v(GraphSP *g_h, int node) {
    vector<DegNode> influenced_nodes;
    vector<int> neigs;
    
    influenced_nodes.push_back(DegNode{node, g_h->degree[node]});
    for (auto key_edge : g_h->edges[node]) {
        int neig = key_edge.first;
        neigs.push_back(neig);
        influenced_nodes.push_back(DegNode{neig, g_h->degree[neig]});
    }

    // add new edges
    for ( int i=0; i<neigs.size(); i++) {
        for (int j=i+1; j<neigs.size(); j++) {
            int u = neigs[i];
            int v = neigs[j];
            if (g_h->edges[u].find(v) == g_h->edges[u].end()) { // no edge
                g_h->edges[u].insert(make_pair(v, edge_sp{u, v, g_h->edges[u][node].length+g_h->edges[node][v].length, g_h->edges[u][node].cost+g_h->edges[node][v].cost }));
                g_h->edges[v].insert(make_pair(u, edge_sp{v, u, g_h->edges[v][node].length+g_h->edges[node][u].length, g_h->edges[v][node].cost+g_h->edges[node][u].cost }));

                g_h->degree[u]++;
                g_h->degree[v]++;
            }
            else {
                int dis = g_h->edges[u][node].length + g_h->edges[node][v].length;
                int cost = g_h->edges[u][node].cost + g_h->edges[node][v].cost;

                if (dis < g_h->edges[u][v].length ) { // smaller distance
                    g_h->edges[u][v] = edge_sp{u, v, dis, cost}; // update, can not use insert.
                    g_h->edges[v][u] = edge_sp{v, u, dis, cost};
                }
                else if ( dis == g_h->edges[u][v].length &&  cost < g_h->edges[u][v].cost ) {
                    // smaller cost
                    g_h->edges[u][v] = edge_sp{u, v, dis, cost}; 
                    g_h->edges[v][u] = edge_sp{v, u, dis, cost};
                    
                }
            }
        }
    }

    // eliminate node
    for (int neig : neigs ) {
        g_h->edges[neig].erase(node);
        g_h->edges[node].erase(neig);
        g_h->degree[neig]--;
        g_h->degree[node]--;
    }
    return influenced_nodes;
}


void H2H::build_h2h_index() {
    DP_tree_decomposition(n); // self.n

    // build position, distance, cost index
    indicators::ProgressBar bar{
        indicators::option::PrefixText{"Build index"},
    };
    indicators::show_console_cursor(false);

    for (int i=n-1; i>=0; i-- ) { // top-down manner
        int node = pi_inv[i]; // node with the largest order is the root
        map<int, int> xi_depth;
        vector<int> ancestor;

        for (int xi=node; xi!=-1; xi=X[xi].fa) {
            xi_depth[xi] = X[xi].depth;
            ancestor.push_back(xi);
        }
        for (auto rit=ancestor.rbegin(); rit != ancestor.rend(); rit++) {
            X[node].anc.push_back( *rit );
        }

        for (auto it=X[node].nodes.begin(); it!=X[node].nodes.end(); it++) {
            X[node].position.push_back( X[ *it ].depth );
        }

        for(int i=0; i<xi_depth[node]; i++) {
            int Dis = inf;
            int Cost = inf;
            int d = 0;
            int c = 0;
            for (int j=0; j<X[node].nodes.size()-1; j++) {
                if (X[node].position[j] > i) {
                    d = X[ X[node].nodes[j] ].distance[i];
                    c = X[ X[node].nodes[j] ].index_cost[i];
                }
                else {
                    d = X[ X[node].anc[i] ].distance[ X[node].position[j] ];
                    c = X[ X[node].anc[i] ].index_cost[ X[node].position[j] ];
                }
                

                if (Dis > d + X[node].phi[j]) {
                    Dis = d + X[node].phi[j];
                    Cost = c + X[node].phi_cost[j];
                } else if (Dis == d + X[node].phi[j] && Cost > c + X[node].phi_cost[j]) {
                    Dis = d + X[node].phi[j];
                    Cost = c + X[node].phi_cost[j];
                }

            }
            X[node].distance.push_back(Dis);
            X[node].index_cost.push_back(Cost);

        }
        X[node].distance.push_back(0);
        X[node].index_cost.push_back(0);

        bar.set_progress((n-i)*100/n);
    }
    indicators::show_console_cursor(true);

}

#endif