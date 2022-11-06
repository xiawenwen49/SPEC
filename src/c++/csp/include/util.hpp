#ifndef _util_hpp_
#define _util_hpp_

#include <iostream>
#include <stdio.h>

#include "fmt/core.h"
#include "common.hpp"
#include "cspp.hpp"

#define __DEBUG

#ifdef __DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
  }
}

#define cuda_err_chk(ans) { gpuAssert((ans), __FILE__, __LINE__); }




template<typename Label_t, typename Edge_t>
void save_labels_on_sink(int u, int v, std::vector<Label_t>& myvector, CSPP<Label_t, Edge_t>* self) {
    // save all non-dominate labels on sink, used for training.
    // the prune strategy in prune_strategy should be 1, to make sure all non-dominate labels on sink will be preserved.
    // all the labels here should be non-dominate labels already.

    assert(self->prune_type == 1);

    string sink_non_dominate_labels_filename = fmt::format("{}_sink_non_dominate_labels_{}_{}.txt", self->output_dir + self->graph_name, u, v);
    ofstream of(sink_non_dominate_labels_filename);
    // for saving all sink's non-dominate labels (for prune_type 1)


    for (auto label : myvector) {
        of << fmt::format("{:d} {:d} {:.0f} {:.0f}", u, v, label.d, label.delay) << endl;
    }

}

template<typename Label_t, typename Edge_t>
void save_csp_path(int source, int sink, Label_t sink_csp_label, std::map<int, std::vector<Label_t>>* dominace_list, CSPP<Label_t, Edge_t>* self ) {
    string csp_path_file = fmt::format("{}_csp_path_{}_{}.txt", self->output_dir+self->graph_name, source, sink);
    // FILE * f = fopen(csp_path_file.c_str(), "w");
    // if (f != NULL)
    // {
    //     fclose(f);
    //     return;
    // }
        
    ofstream csp_path_of(csp_path_file);




    Label_t current = sink_csp_label;
    int father = sink_csp_label.father;
    std::vector<Label_t> path;
    path.push_back(current); // path end point is at the 1st line of saved file.

    // printf("\nsave_csp_path:\n");

    // printf("sink_csp_label: vid %d d %.0f c %.0f fa %d\n", sink_csp_label.vid, sink_csp_label.d, sink_csp_label.delay, sink_csp_label.father);
    // printf("father: %d\n", father);
    // printf("hop(type): %d\n", sink_csp_label.hop);
    bool need_intermediate = false;
    if (need_intermediate){
    while (current.vid != source )
    {
        
        Edge * e_;
        bool find_edge = false;
        for (auto e : self->g->v_l[father].edges )
        {
            if (e->snk == current.vid)
            {
                e_ = e;
                find_edge = true;
                break;
            }

        }
        // printf("find edge: %d\n", find_edge);
        // if (find_edge)
        //     printf("edge: len %.0f cost %.0f\n", e_->len, e_->delay);


        bool find_label = false;
        for ( auto label : (*dominace_list)[father])
        {

            if (!find_edge)
            {
                QueryRes result_min_dis = self->h2h_solver->query(label.vid, sink_csp_label.vid);
                QueryRes result_min_cost = self->h2h_solver_cost->query(label.vid, sink_csp_label.vid); // min cost, max distance
                // printf("dis res: d %.0f c %.0f\n", label.d + result_min_dis.distance, label.delay + result_min_dis.cost);
                // printf("cos res: d %.0f c %.0f\n", label.d + result_min_cost.distance, label.delay + result_min_cost.cost);
                // printf("\n");

                if ( (label.d + result_min_dis.distance == current.d && label.delay + result_min_dis.cost == current.delay) ||
                    (label.d + result_min_cost.distance == current.d && label.delay + result_min_cost.cost == current.delay)
                )
                {
                    path.push_back(label);
                    current = label;
                    father = current.father;
                    find_label = true;
                    break;
                }


            }
            else 
            {
                if ( label.d + e_->len == current.d && label.delay + e_->delay == current.delay )
                {
                    path.push_back(label);
                    current = label;
                    father = current.father;
                    find_label = true;
                    break;
                }
            }
            

        }
        if (!find_label)
        {
            // printf("not find a label\n");
            // printf("current vid: %d fa: %d\n", current.vid, father);
            // printf("father node labels: \n");
            // for ( auto label : (*dominace_list)[father])
            // {
            //     printf("vid: %d, d: %.0f, c: %.0f, d_est: %.0f, hop(type): %d\n", label.vid, label.d, label.delay, label.distance_estimated, label.hop);
            // }
            exit(0);
        }
        else
        {
            // printf("current vid: %d fa: %d\n", current.vid, father);

        }

    }

    }

    // path.push_back(current);

    csp_path_of<<std::fixed<<std::setprecision(0);
    for (auto label : path)
        csp_path_of << label.vid << " " << label.d << " " << label.delay << endl;
    
    std::cout<< "csp path saved: " << csp_path_file << endl;

}

// struct set_labels_comp
// {
//     bool operator()(std::tuple<int, int, int> a, std::tuple<int, int, int> b)
//     {
//         return std::get<1>(a) < std::get<1>(b);
//     }
//     // comp is an object of this type and a and b are key values, shall return true if a is considered to go before b
// };

// bool set_labels_comp(std::tuple<int, int, int> a, std::tuple<int, int, int> b)
// {
    // return std::get<1>(a) < std::get<1>(b);
// }


std::set<std::tuple<int, int, int>, set_labels_comp> load_csp_path(string csp_path) {
    FILE * f = fopen(csp_path.c_str(), "r");
    if (f == NULL)
        throw std::runtime_error(std::strerror(errno));
    
    
    int vid, d, c;

    std::set<std::tuple<int, int, int>, set_labels_comp> set_labels;
    while (fscanf(f, "%d %d %d\n", &vid, &d, &c)!=EOF) 
    {
        set_labels.insert( std::make_tuple(vid, d, c) );
    }
    fclose(f);
    return set_labels;

}

template<typename Label_t>
int check_frontier_first_csp_label(std::set<std::tuple<int, int, int>, set_labels_comp>* set_labels, std::priority_queue<Label_t, std::vector<Label_t>, label_comp_<Label_t> >* frontiers) {
    // find the first position in frontiers which is a label on the real csp path.

    std::vector<Label_t> cache;
    int position = 0;
    while (!frontiers->empty()) 
    {
        Label_t top = frontiers->top();
        cache.push_back(top);
        frontiers->pop();

        if ( set_labels->find( std::make_tuple(top.vid, (int)top.d, (int)top.delay) ) != set_labels->end() )
        {
            break;
        }
        position++;
    }


    for (auto label : cache ) 
    {
        frontiers->push(label);
    }

    return position;


}



#endif

