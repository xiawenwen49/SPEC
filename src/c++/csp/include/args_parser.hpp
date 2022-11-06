#include "cxxopts.hpp"

cxxopts::ParseResult parse_args(int argc, char ** argv) {
    static cxxopts::Options options("csp", "Constrained shortest path");
    options.add_options()
    // ("prune,debug", "Enable debugging") // a bool parameter
    ("prune", "Prune strategy type", cxxopts::value<int>()->default_value("0") )
    ("config", "Config file name", cxxopts::value<std::string>()->default_value(" ") )
    ("query_file", "Query file name", cxxopts::value<std::string>()->default_value("QUERY_FILE") )
    ("graph_file", "Graph file name", cxxopts::value<std::string>()->default_value("GRAPH_FILE") )
    ("graph_type", "Graph type", cxxopts::value<int>()->default_value("2") )
    ("ntask", "Number of tasks (queries)", cxxopts::value<int>()->default_value("-1") )
    ("multithread", "use multithread or not", cxxopts::value<int>()->default_value("1"))
    ("withtruth", "Set the truth csp label from the beginning or not ", cxxopts::value<int>()->default_value("0") )
    ("truth_position", "Compute truth csp label positions in frontier or not", cxxopts::value<int>()->default_value("0") )
    // ("v", "Verbose output", cxxopts::value<bool>()->default_value("false"))
    ("rl_expand_control", "Use rl controller for expand num or not", cxxopts::value<int>()->default_value("0") )
    ("default_expand_num", "Default expand num if not use rl controller", cxxopts::value<int>()->default_value("100") )
    ;
    
    cxxopts::ParseResult result = options.parse(argc, argv);

    return result;
}


