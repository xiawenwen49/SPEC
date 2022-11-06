#include <stdio.h>
#include <chrono>
#include <string>

#include "../third_libs/cxxopts.hpp"

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
    ("d_c_inverse", "Distance-cost inverse to build inverse index", cxxopts::value<int>()->default_value("0") )
    ("query_file", "Query file name", cxxopts::value<std::string>()->default_value("REPO_DIR/ConstrainedSP/inputs/rquery_ne_s2") )
    ("graph_file", "Graph file name", cxxopts::value<std::string>()->default_value("REPO_DIR/ConstrainedSP/inputs/ne.graph") )
    ("graph_type", "Graph type", cxxopts::value<int>()->default_value("2") )
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


    // read graph
    GraphSP * g = new GraphSP();
    // bool d_c_inverse = true;
    read_graph_sp(graph_file.c_str(), g, graph_type, d_c_inverse);

    // read query
    Task *tasks = new Task();
    tasks->load_query(query_file.c_str());

    // h2h
    H2H solver;
    solver.init(g);

    string index_file(graph_file);
    printf("graph file: %s\n", graph_file.c_str());
    printf("d_c_inverse: %d.\n", d_c_inverse);
    if (!d_c_inverse)
        index_file.append("_h2h_index.txt");
    else
        index_file.append("_h2h_index_d_c_reverse.txt");
    std::cout << "Index file: " << index_file << endl;

    high_resolution_clock::time_point start_time_ = high_resolution_clock::now();
    solver.build_h2h_index();
    high_resolution_clock::time_point end_time_ = end_time_ = high_resolution_clock::now();
    long long time = std::chrono::duration_cast<milliseconds>(end_time_ - start_time_).count();

    solver.save_index(index_file.c_str());

    printf("Build index time (milliseconds): %lld milliseconds\n", time);


}