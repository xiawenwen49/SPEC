#ifndef _cspp_hpp_
#define _cspp_hpp_
#include <vector>
#include "task.hpp"
#include "label.hpp"
#include "constraint.hpp"
#include "graph.hpp"
#include "predict.hpp"
#include "common.hpp"

#include "../sp/h2h.hpp"
#include "../sp/graph_sp.hpp"
#include "../sp/utils_sp.hpp"


template<typename Label_t>
struct QueryContext {
    int u;
    int v;
    float cost_limit;
    float minimal_distance;
    float global_best_dis;
    bool csp_truth_file_exist;
    int steps;
    int cumulated_time; // us (macros)
    int repeat=0;


    std::priority_queue<Label_t, std::vector<Label_t>, label_comp_<Label_t> > *frontiers = NULL;
    std::map<int, std::vector<Label_t>> * dominace_list = NULL;;

    void clear() {
        if (frontiers != NULL)
        {
            delete frontiers;
            frontiers = NULL; // IMPORTANT
        }
            
        if (dominace_list != NULL)
        {
            delete dominace_list;
            dominace_list = NULL; // IMPORTANT
        }
            
    }
};

c_edge *create_edge_list(Graph<c_edge> *g) {
    c_edge *l = (c_edge *) malloc(sizeof(c_edge) * g->M);
    for (int i = 0; i < g->M; i++) {
    l[i].len = g->e_l[i]->len;
    l[i].delay = g->e_l[i]->delay;
    }

    return l;
}

template<typename Label_t, typename Edge_t>
class CSPP {
    using Graph_t = Graph<Edge_t>;
    using Constraint_t = c_constraint<Label_t>;
    public:
    int *path;
    H2H * h2h_solver;
    H2H * h2h_solver_cost;
    Net * dqn;
    int prune_type;
    int multi_thread;
    int withtruth;
    int truth_position;
    QueryContext<Label_t> query_context;


    vector<Label_t> init_state;
    vector<int> sink_l;
    
    vector<int *> path_l;
    vector<int> size_of_result_l;
    float global_best_dis = 100000000;


    Graph_t *g;
    string graph_file;
    string graph_name;
    int graph_type;
    string query_file;
    int ntask;
    
    
    vector<Constraint_t> C_l;
    vector<long long> execution_time;

    int total_expanded_labels = 0;
    int prediction_nums = 0;

    // ofstreams
    string output_dir;
    string each_iter_expanded_labels;
    string sink_non_dominate_labels_filename = string("OURPUT_DIR") + this->graph_name + string("_sink_non_dominate_labels.txt");
    
    ofstream each_iter_expanded_labels_of;
    string each_iter_frontier_size;

    string csp_path;

    int rl_expand_control; // 1, 0
    int default_expand_num;


    public:
    std::vector<int> reset(int, int, float);
    std::vector<int> one_iteration(int expand_num);
    void solve_one_query(int u, int v, float cost_limit);
    int RUN_CSPP_CPU_with_priority();
    int RUN_CSPP_CPU_with_priority_thread();
    
    std::tuple<std::vector<Label_t>, bool> prune_strategy(int sink, Label_t * label_node, int prune_type, float cost_limit, std::map<int, std::vector<Label_t>>* dominace_list);
    void add_initial_label(std::map<int, std::vector<Label_t>> * dominace_list, int source, int sink);


    CSPP(string graph_file, string graph_name, int graph_type, string query_file, int ntask,
        int prune_type, int multi_thread, int withtruth, int truth_position, int rl_expand_control, int default_expand_num):
        graph_file(graph_file), graph_name(graph_name), graph_type(graph_type), query_file(query_file), ntask(ntask),
        prune_type(prune_type), multi_thread(multi_thread), withtruth(withtruth), truth_position(truth_position),
        rl_expand_control(rl_expand_control), default_expand_num(default_expand_num)
        {

        this->read_graph_and_queries();
        printf("read graph and query finished\n");

        // set dqn net
        c10::InferenceMode guard(true); // inference mode
        this->dqn = new Net(8, 64, 7);
        string script_module = string("FILE_DIR_OF_THE_TRACED_Q_NET_BY_RL_TRAINING");

        this->dqn->load_from_scriptmodule(script_module.c_str());

        // set h2h index
        this->create_h2h_solver();

        this->set_ofstreams();
        
    }

    void read_graph_and_queries()
    {
        using Graph = Graph<Edge_t>;
        using Constraint = c_constraint<Label_t>;

        // read graph file
        assert(this->graph_type == 2);
        this->g = new Graph();
        read_graph(this->graph_file.c_str(), this->g, this->graph_type);
        this->g->adj_to_csr();
        this->g->el = create_edge_list(g);

        // read query file
        task *tasks = new task();
        tasks->load_query(this->query_file.c_str());

        for (int i = 0; i < ntask; i++) {
            Label_t label(tasks->source_l_[i]);
            this->init_state.push_back(label);

            Constraint c(tasks->delay_l_[i]);
            this->C_l.push_back(c);
        }

        // set sink_l
        this->sink_l = tasks->sink_l_;

        if (this->ntask == -1)
            this->ntask = tasks->ntask;

        std::cout << "Number of V " << g->N << std::endl;
        std::cout << "Number of E " << g->M << std::endl;


    }
    

    void set_ofstreams() {
        output_dir = string("OUTPUT_DIR");
        each_iter_expanded_labels = output_dir + this->graph_name + string("_each_iter_expanded_labels.txt");
        each_iter_frontier_size = output_dir + this->graph_name + string("_each_iter_frontier_size.txt");
        each_iter_expanded_labels_of.open(each_iter_expanded_labels.c_str());

    }

    void write_to_file(ofstream ofs, string str) {
        ofs << str;
    }


    int solve_cpu() {
        this->RUN_CSPP_CPU_with_priority_thread();
        return 0;
    }



    void create_h2h_solver() {
        GraphSP * g = new GraphSP();
        GraphSP * g_inv = new GraphSP();
        const char *graph_f = this->graph_file.c_str();

        read_graph_sp(graph_f, g, this->graph_type, false);
        read_graph_sp(graph_f, g_inv, this->graph_type, true);


        this->h2h_solver = new H2H();
        this->h2h_solver_cost = new H2H();


        this->h2h_solver->init(g);
        this->h2h_solver_cost->init(g_inv);

        string index_file = this->graph_file + string("_h2h_index.txt");
        string index_file_cost = this->graph_file + string("_h2h_index_d_c_reverse.txt");

        if (!exists(index_file)) {
            std::cout << "Index file not found." <<std::endl;
            exit(0);
        }
        else { // load the existing index if it exists.
            this->h2h_solver->load_index_multi_thread(index_file.c_str());
        }

        

        if (!exists(index_file_cost)) {
            std::cout << "Index file not found." <<std::endl;
            exit(0);
        }
        else { // load the existing index if it exists.
            this->h2h_solver_cost->load_index_multi_thread(index_file_cost.c_str());
        }

    }




};

#endif
