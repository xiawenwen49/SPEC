#include <stdio.h>
#include <iostream>
#include <vector>
#include <csignal>
#include <assert.h>
#include <experimental/filesystem>
#include <cuda_runtime.h>
#include "include/comm.hpp"
#include "include/task.hpp"
#include "include/label.hpp"
#include "include/constraint.hpp"
#include "include/args_parser.hpp"


// #include "../third_libs/cxxopts.hpp"

#ifndef CSP
#define CSP 1
#endif


using namespace std;

int main(int argc, char *argv[]) {
    
    cxxopts::ParseResult args = parse_args(argc, argv);
    int prune_type = args["prune"].as<int>();
    
    string config_file = args["config"].as<string>();
    string graph_file = args["graph_file"].as<string>();
    string query_file = args["query_file"].as<string>();
    int graph_type = args["graph_type"].as<int>();
    int ntask = args["ntask"].as<int>();
    int multi_thread = args["multithread"].as<int>();
    int withtruth = args["withtruth"].as<int>();
    int truth_position = args["truth_position"].as<int>();
    int rl_expand_control = args["rl_expand_control"].as<int>();
    int default_expand_num = args["default_expand_num"].as<int>();


    if (config_file != " ") {
        string graph_type_str;
        string ntask_str;
        
        ifstream conf_if (config_file.c_str());
        conf_if >> graph_type;
        conf_if >> graph_file;
        conf_if >> query_file;
        conf_if >> ntask_str;
        
        graph_type = stoi(graph_type_str);
        ntask = stoi(ntask_str);
        conf_if.close();

    }


    cout << "Graph file: " << graph_file << endl;
    cout << "Query file: " << query_file << endl;
    cout << "Prune type: " << prune_type << endl;
    cout << "Use multithread: " << multi_thread << endl;
    cout << "With csp truth: " << withtruth << endl;
    cout << "Compute truth label position: " << truth_position << endl;

    string graph_name = graph_file.substr( graph_file.find_last_of("/\\")+1 );


    CSPP<c_label, c_edge> *solver;

    solver = new CSPP<c_label, c_edge>(graph_file, graph_name, graph_type, query_file, ntask,
            prune_type, multi_thread, withtruth, truth_position, rl_expand_control, default_expand_num);
    
    solver->solve_cpu();
    return 0;
}
