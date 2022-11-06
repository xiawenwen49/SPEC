#include <stdio.h>
#include <chrono>
#include <string>

#include "third_libs/cxxopts.hpp"

#include "h2h.hpp"
#include "graph_sp.hpp"
#include "utils_sp.hpp"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::nanoseconds;

cxxopts::ParseResult parse_args(int argc, char ** argv) {
    static cxxopts::Options options("csp", "Constrained shortest path");
    options.add_options()
    // ("prune,debug", "Enable debugging") // a bool parameter
    ("d_c_inverse", "Distance-cost inverse to build inverse index", cxxopts::value<int>()->default_value("0") )
    ("query_file", "Query file name", cxxopts::value<std::string>()->default_value("/home/xiawenwen/workspace/ConstrainedSP/inputs/queries/test_sp") )
    ("graph_file", "Graph file name", cxxopts::value<std::string>()->default_value("/home/xiawenwen/workspace/ConstrainedSP/inputs/processed_graphs/ne.graph") )
    ("graph_type", "Graph type", cxxopts::value<int>()->default_value("2") )
    ("ntask", "Num queries", cxxopts::value<int>()->default_value("1") )
    // ("v", "Verbose output", cxxopts::value<int>()->default_value("0"))
    ;
    
    cxxopts::ParseResult result = options.parse(argc, argv);

    return result;
}


int main (int argc, char *argv[]) {
    cxxopts::ParseResult args = parse_args(argc, argv);
    int d_c_inverse = args["d_c_inverse"].as<int>();
    string graph_file = args["graph_file"].as<string>();
    string query_file = args["query_file"].as<string>();
    int graph_type = args["graph_type"].as<int>();
    int ntasks = args["ntask"].as<int>();


    std::cout << "Graph file: " << graph_file << std::endl;
    std::cout << "Query file: " << query_file << std::endl;
    std::cout << "Query num: " << ntasks << std::endl;

    // read graph
    GraphSP * g = new GraphSP();
    read_graph_sp(graph_file.c_str(), g, graph_type, (bool)d_c_inverse);

    // read query
    Task *tasks = new Task();
    tasks->load_query(query_file.c_str());

    // h2h
    H2H solver;
    solver.init(g);

    string index_file(graph_file);
    if (!d_c_inverse)
        index_file.append("_h2h_index.txt");
    else
        index_file.append("_h2h_index_d_c_reverse.txt");

    if (!exists(index_file)) {
        std::cout << index_file << " not exists" << std::endl;
        exit(1);
    }
    else { // load the existing index if it exists.
        solver.load_index_multi_thread(index_file.c_str());
    }


    int u, v, cost;
    QueryRes result;
    for (int i=0; i<tasks->ntask; i++) {
        
        u = tasks->source_l_[i];
        v = tasks->sink_l_[i];
        cost = tasks->cost_l_[i];

        high_resolution_clock::time_point start_time_ = high_resolution_clock::now();
        result = solver.query(u, v);

        high_resolution_clock::time_point end_time_ = end_time_ = high_resolution_clock::now();
        long long time_us = std::chrono::duration_cast<microseconds>(end_time_ - start_time_).count();
        

        printf("query: %d\n", i);
        printf("Node: %d %d.\n", u, v);
        printf("Min length (main key): %d, cost (second key): %d.\n", result.distance, result.cost);
        printf("Time (us): %.2f macros\n", (float)time_us);
        printf("\n");
        
    }







}