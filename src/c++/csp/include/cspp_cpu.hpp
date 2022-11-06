#ifndef _cspp_cpu_hpp_
#define _cspp_cpu_hpp_
// #include <boost>
#include <vector>
#include <map>
#include <cmath>
#include <mutex>
#include <random>
#include <stdlib.h>

#include "label.hpp"
#include "constraint.hpp"
#include "edge.hpp"
#include "graph.hpp"
#include "TimeMeasurer.hpp"
#include "graph.hpp"


#include "cspp.hpp"
#include "util.hpp"

#include "../../sp/h2h.hpp"
// #include <omp.h>

#define LRG_FLOAT 1000000

#ifndef _thread_pool_
#define _thread_pool_
#define thread_pool_size 11
ThreadPool thd_pool(thread_pool_size);
#endif



// template<typename Label_t>
// void add_new_frontier(std::map<int, std::vector<Label_t>> &next_round, Label_t &new_label) {
//     if (next_round.find(new_label.vid) != next_round.end()) 
//     {
//         bool dominate = false;
//         for (auto it = next_round[new_label.vid].begin(); it != next_round[new_label.vid].end();) {
//             if (Label_t::dominate_cpu(*it, new_label)) { // new label is dominated
//                 dominate = true;
//                 break;
//             }
//             if (Label_t::dominate_cpu(new_label, *it)) { // new label dominates some other labels
//                 it = next_round[new_label.vid].erase(it);
//             } else {
//                 it++;
//             }
//         }
//         if (dominate) return;

//         next_round[new_label.vid].push_back(new_label);
//     } 
//     else 
//     {
//         std::vector<Label_t> new_frontier;
//         new_frontier.push_back(new_label);
//         next_round[new_label.vid] = new_frontier;
//     }
// }

// template<typename Label_t>
// void add_new_frontier(std::map<int, std::vector<Label_t>> &next_round, Label_t &new_label, bool valid) {
//     if (next_round.find(new_label.vid) != next_round.end()) 
//     {
//         bool dominate = false;
//         for (auto it = next_round[new_label.vid].begin(); it != next_round[new_label.vid].end();it++) 
//         {
//             if (it->valid == false) continue;

//             if (Label_t::dominate_cpu(*it, new_label)) 
//             { // new label is dominated
//                 dominate = true;
//                 break;
//             }
            
//             if (Label_t::dominate_cpu(new_label, *it)) 
//             { // new label dominates some other labels
//                 // it = next_round[new_label.vid].erase(it);
//                 it->valid = false;
//             }
//         }
//         if (dominate) return;

//         next_round[new_label.vid].push_back(new_label);
//     } 
//     else 
//     {
//         std::vector<Label_t> new_frontier;
//         new_frontier.push_back(new_label);
//         next_round[new_label.vid] = new_frontier;
//     }
// }

// template<typename Label_t>
// void add_new_frontier(std::map<int, std::vector<Label_t>> *next_round, Label_t &new_label, bool valid) {
//     if ((*next_round).find(new_label.vid) != (*next_round).end()) 
//     {
//         bool dominate = false;
//         for (auto it = (*next_round)[new_label.vid].begin(); it != (*next_round)[new_label.vid].end();it++) 
//         {
//             if (it->valid == false) continue;

//             if (Label_t::dominate_cpu(*it, new_label)) 
//             { // new label is dominated
//                 dominate = true;
//                 break;
//             }
            
//             if (Label_t::dominate_cpu(new_label, *it)) 
//             { // new label dominates some other labels
//                 // it = next_round[new_label.vid].erase(it);
//                 it->valid = false;
//             }
//         }
//         if (dominate) return;

//         (*next_round)[new_label.vid].push_back(new_label);
//     } 
//     else 
//     {
//         std::vector<Label_t> new_frontier;
//         new_frontier.push_back(new_label);
//         (*next_round)[new_label.vid] = new_frontier;
//     }
// }

template<typename Label_t>
void add_new_frontier(std::map<int, std::vector<Label_t>> *next_round, Label_t new_label) {
    if ((*next_round).find(new_label.vid) != (*next_round).end()) 
    {
        bool dominate = false;
        for (auto it = (*next_round)[new_label.vid].begin(); it != (*next_round)[new_label.vid].end();it++) 
        {
            if (it->valid == false) continue;

            if (Label_t::dominate_cpu(*it, new_label)) 
            { // new label is dominated
                dominate = true;
                break;
            }
            
            if (Label_t::dominate_cpu(new_label, *it)) 
            { // new label dominates some other labels
                // it = (*next_round)[new_label.vid].erase(it);
                it->valid = false;
            }
        }
        if (dominate) return;

        (*next_round)[new_label.vid].push_back(new_label);
    } 
    else 
    {
        std::vector<Label_t> new_frontier;
        new_frontier.push_back(new_label);
        (*next_round)[new_label.vid] = new_frontier;
    }
}

template<typename Label_t, typename Edge_t>
std::tuple<std::vector<Label_t>, bool> CSPP<Label_t, Edge_t>::prune_strategy(int sink, Label_t * label_node, int prune_type, float cost_limit, std::map<int, std::vector<Label_t>>* dominace_list) {
    std::vector<Label_t> sink_label_finded;

    int node = label_node->vid;

    // assert(prune_type == 0); // constrained to 0 now.
    this->total_expanded_labels++;

    if (prune_type == 0) {
        // only preserve the best label on sink
        
        if ( node != sink && label_node->d >= this->query_context.global_best_dis )
            return std::make_tuple(sink_label_finded, false); // false: not preserve this label
        
        QueryRes result_min_cost = this->h2h_solver_cost->query(node, sink); // min cost, max distance
        QueryRes result_min_dis = this->h2h_solver->query(node, sink); // min distance, max cost
        // float distance_min_cost = (float) result_min_cost.distance;
        // float cost_min_cost = (float) result_min_cost.cost;
        float distance_min_cost = (float) result_min_cost.cost; 
        float cost_min_cost = (float) result_min_cost.distance; // 'distance' is always the main key! hence here 'distance' is actually cost.


        float distance_min_dis = (float) result_min_dis.distance;
        float cost_min_dis = (float) result_min_dis.cost;
        
        // /* // with, without prune

        if ( distance_min_dis + label_node->d >= this->query_context.global_best_dis)
            return std::make_tuple(sink_label_finded, false);

        if ( cost_min_cost + label_node->delay > cost_limit)
            return std::make_tuple(sink_label_finded, false);
        

        if ( distance_min_dis + label_node->d < this->query_context.global_best_dis && cost_min_dis + label_node->delay <= cost_limit) {
            // thd_pool_mtx_list[sink%mtx_num].lock(); // problem... thread shared variable
            // (*dominace_list)[sink].push_back( Label_t(sink, distance_min_dis + label_node->d,  cost_min_dis + label_node->delay ) );
            // thd_pool_mtx_list[sink%mtx_num].unlock();
            sink_label_finded.push_back( Label_t(sink, distance_min_dis+label_node->d,  cost_min_dis+label_node->delay, distance_min_dis+label_node->d, label_node->vid, -1 ) );
            
            thd_pool_mtx.lock();
            this->query_context.global_best_dis = distance_min_dis + label_node->d; // problem... thread shared variable
            thd_pool_mtx.unlock();

            // return std::make_tuple(sink_label_finded, false);
            return std::make_tuple(sink_label_finded, true); // here return true to preserve labels on csp into the dominace_list...
        }
    
        // thd_pool_mtx.lock();
        this->prediction_nums++;
        // thd_pool_mtx.unlock();
        
        if ( distance_min_cost + label_node->d < this->query_context.global_best_dis && cost_min_cost + label_node->delay <= cost_limit ) {
            // thd_pool_mtx_list[sink%mtx_num].lock(); // problem... thread shared variable
            // (*dominace_list)[sink].push_back( Label_t(sink, distance_min_cost + label_node->d,  cost_min_cost + label_node->delay ) );
            // thd_pool_mtx_list[sink%mtx_num].unlock();

            sink_label_finded.push_back( Label_t(sink, distance_min_cost+label_node->d,  cost_min_cost+label_node->delay, distance_min_dis+label_node->d, label_node->vid, -2 ) );
            
            thd_pool_mtx.lock();
            this->query_context.global_best_dis = distance_min_cost + label_node->d; // problem... thread shared variable
            thd_pool_mtx.unlock();
            

        }

        // */ // with, without prune
        label_node->priority = 0;
        label_node->distance_estimated = label_node->d + distance_min_dis; // default


        return std::make_tuple(sink_label_finded, true);

    }
    else if (prune_type == 1) {
        // preserve all non-dominate labels on the sink
        
        if ( (*dominace_list).find(sink) != (*dominace_list).end() ) {
            Label_t l_sink_best = *std::min_element((*dominace_list)[sink].begin(), (*dominace_list)[sink].end(), min_distance<Label_t>);

            // if ( node != sink && label_node->d >= l_sink_best.d )
            //     return false; // false: not preserve this label
            
            if ( Label_t::dominate_cpu(l_sink_best, *label_node) )
                return std::make_tuple(sink_label_finded, false);
        }
        
        label_node->priority = 0; // just for priority setting
        label_node->distance_estimated = label_node->d; // just for priority setting
        return std::make_tuple(sink_label_finded, true);

 
    }
    else if (prune_type == 2) {
        /* code */
        ;
    }
    return std::make_tuple(sink_label_finded, true);

}


template<typename Label_t, typename Edge_t>
void CSPP<Label_t, Edge_t>::add_initial_label(std::map<int, std::vector<Label_t>> * dominace_list, int source, int sink) {
    QueryRes result_tmp = this->h2h_solver_cost->query(source, sink);
    QueryRes result; result.distance = result_tmp.cost; result.cost = result_tmp.distance; // !!!!!!!!!!! VERY IMPORTANT
    // h2h_solver_cost's distance represents COST, and its cost represents DISTANCE!!!
    // The same rule also lies in prune_strategy FUNCTION!!!

    QueryRes result_min_dis = this->h2h_solver->query(source, sink);
    std::vector<Label_t> new_dominace_vec;
    
    Label_t sink_initial = Label_t(sink, result.distance, result.cost );
    // printf("%d %d %d %f %f\n", sink, result.distance, result.cost, sink_initial.d, sink_initial.delay );
    // exit(0);
    sink_initial.father = -111;
    new_dominace_vec.push_back( sink_initial );

    (*dominace_list)[sink] = new_dominace_vec;
    // printf("%d %f %f\n", (*dominace_list)[sink][0].vid, (*dominace_list)[sink][0].d, (*dominace_list)[sink][0].delay );
    // exit(0);
    this->query_context.global_best_dis = result.distance;
    this->query_context.minimal_distance = result_min_dis.distance;

}


template<typename Label_t>
std::vector<Label_t> * pop_labels(std::priority_queue<Label_t, std::vector<Label_t>, label_comp_<Label_t> >* frontiers, int n) {
    std::vector<Label_t> * store = new std::vector<Label_t> (n);
    for (int i=0; i<n; i++) {
        (*store)[i] = frontiers->top();
        frontiers->pop();
    } 
    return store;
}


template<typename Label_t>
void free_heap(std::vector<std::vector<Label_t> *> stores) {
    for (auto store : stores)
        delete store;
}

template<typename Label_t>
void free_heap(std::vector< std::map<int, std::vector<Label_t> > * > stores) {
    for (auto store : stores)
        delete store;
}

template<typename Label_t>
void print_csp(std::vector<Label_t> * myvector) {
    for (auto label : *myvector ){
        if (label.valid)
            printf("found labels: vid %d d %.2f cost %.2f\n", label.vid, label.d, label.delay);
            // printf("found labels: vid %d d %.2f cost %.2f hop_id %d fa_id %d\n", label.vid, label.d, label.delay, label.hop, label.father);
    }

}



template<typename Label_t, typename Edge_t>
std::tuple<std::map<int, std::vector<Label_t>> *, std::vector<Label_t>> expand_worker(CSPP<Label_t, Edge_t> *self,  std::vector<Label_t> * labels, std::map<int, std::vector<Label_t>> * dominace_list, int source, int sink, float cost_limit) {
    

    std::map<int, std::vector<Label_t>> * next_round = new std::map<int, std::vector<Label_t>>;
    // int pruned_num = 0;
    // int sink = self->sink_l[taskid];
    // float cost_limit = self->C_l[taskid].delay_L;

    std::map<int, std::vector<Label_t>> local_dominace_list;

    std::vector<Label_t> sink_labels_total;
    std::vector<Label_t> sink_labels;
    bool preserve;

    if (labels->size() == 0 )
    {
        return std::make_tuple(next_round, sink_labels_total);
    }

    for (auto frontier : *labels) {
        // if (frontier.distance_estimated > self->global_best_dis) // here the distance_estimated must be label.d + distance_min_dis in prune_strategy().
        // {
        //     continue;
        // }

        for (auto e : self->g->v_l[frontier.vid].edges) {
            Label_t new_label;
            frontier.expand_cpu(e->snk, *e, &new_label);
            // if (!self->C_l[taskid].pass_constraints_check_cpu(new_label))  continue;
            if (!(new_label.delay<=cost_limit))  continue; // must have the brackets

            bool dominate = false;
            if ((*dominace_list).find(new_label.vid) != (*dominace_list).end()) 
            { // check dominace on that new vertex
                // bool dominate = false;
                for (auto it = (*dominace_list)[new_label.vid].begin(); it != (*dominace_list)[new_label.vid].end(); it++ ) 
                {
                    if (it->valid && Label_t::dominate_cpu(*it, new_label)) 
                    { // new label is dominated
                        dominate = true;
                        break;
                    }
                    
                    // if (Label_t::dominate_cpu(new_label, *it)) 
                    // { // new label dominates some other labels
                        // it->replace(new_label);
                        // continue;
                        // it = dominace_list[new_label.vid].erase(it); //  erase returns the element (an iterator) that follows the last element removed.
                    // } 
                    
                }

                // if (dominate) continue; // go to next edge

                // thd_pool_mtx_list[new_label.vid % mtx_num].lock();
                // (*dominace_list)[new_label.vid].push_back(new_label); // problem... thread shared variable
                // thd_pool_mtx_list[new_label.vid % mtx_num].unlock();

                // add_new_frontier(next_round, new_label);

            } 

            if (dominate) continue;

            if (local_dominace_list.find(new_label.vid) != local_dominace_list.end()) 
            {
                for (auto it = local_dominace_list[new_label.vid].begin(); it != local_dominace_list[new_label.vid].end(); it++ ) 
                {
                    if (it->valid && Label_t::dominate_cpu(*it, new_label)) 
                    { // new label is dominated
                        dominate = true;
                        break;
                    }
                    
                    if (Label_t::dominate_cpu(new_label, *it)) 
                    { // new label dominates some other labels
                        it->replace(new_label);
                        continue;
                        // break;
                    }
                }
            }
            else
            {
                add_new_frontier(&local_dominace_list, new_label);
            }


            if (dominate) continue;

            
            std::tie(sink_labels, preserve) = self->prune_strategy(sink, &new_label, self->prune_type, cost_limit, dominace_list);
            if (sink_labels.size() > 0)
                sink_labels_total.insert(sink_labels_total.end(), sink_labels.begin(), sink_labels.end());

            if ( !preserve )
            {
                // pruned_num++;
                continue;
            }

            add_new_frontier(next_round, new_label);
                        

            // unnecessary, because prune_strategy will update global_best_dis
            // if ( new_label.vid == sink &&  new_label.d < self->global_best_dis ) 
            // {   
            //     thd_pool_mtx.lock();
            //     self->global_best_dis = new_label.d;
            //     thd_pool_mtx.unlock();
            // }



        }
    }

    return std::make_tuple(next_round, sink_labels_total);

    

}

template<typename Label_t, typename Edge_t>
std::vector<int> CSPP<Label_t, Edge_t>::reset(int u, int v, float cost_limit) {
    // this->query_context.clear();

    this->query_context.repeat += 1;
    this->query_context.u = u;
    this->query_context.v = v;
    this->query_context.cost_limit =  cost_limit;
    this->query_context.global_best_dis = 100000000;
    
    this->query_context.clear(); // IMPORTANT
    this->query_context.frontiers = new std::priority_queue<Label_t, std::vector<Label_t>, label_comp_<Label_t> >;
    this->query_context.dominace_list = new std::map<int, std::vector<Label_t>>;

    this->query_context.steps = 0;
    this->query_context.cumulated_time = 0;
    

    Label_t src(u);
    this->query_context.frontiers->push(src);
    this->add_initial_label(this->query_context.dominace_list, u, v); // on sink
    add_new_frontier(this->query_context.dominace_list, src); // on source

    // printf("%d %f %f\n", (*(this->query_context.dominace_list))[v][0].vid, (*(this->query_context.dominace_list))[v][0].d, (*(this->query_context.dominace_list))[v][0].delay );
    // exit(0);

    // load csp_truth file
    if (this->withtruth){
        try {
            string csp_path_file = fmt::format("{}_csp_path_{}_{}.txt", this->output_dir+this->graph_name, u, v);
            std::set<std::tuple<int, int, int>, set_labels_comp> set_labels;
            set_labels = load_csp_path(csp_path_file);
            // push groundtruth label from the begining
            auto best_label = *(set_labels.rbegin());
            (*(this->query_context.dominace_list))[this->query_context.v].push_back( Label_t( std::get<0>(best_label), std::get<1>(best_label), std::get<2>(best_label)) );
            this->query_context.global_best_dis = std::get<1>(best_label);
            printf("groundtruth set: vid %d d %d c %d\n", std::get<0>(best_label), std::get<1>(best_label), std::get<2>(best_label));
            
        }
        catch ( std::runtime_error& e ) {
            printf("not found csp file\n");
            printf("groundtruth not set\n");
        }
    }
    




    return std::vector<int>{0,
        0,
        (int)(this->query_context.cost_limit),
        (int)(this->query_context.minimal_distance),
        1, // sign for start
        0,
        (int)(this->query_context.global_best_dis),
        (int)(this->query_context.global_best_dis),
        0, // sign for start
        0, // sign for start
        0, 
        (int)(this->query_context.cost_limit), // sign for start
        0, // reward, useless for reset
    };


    
}

template<typename Label_t, typename Edge_t>
std::vector<int> CSPP<Label_t, Edge_t>::one_iteration(int expand_num__) { // step function
    this->query_context.steps += 1;
    printf("epi %d, step %d, expand_num %d, ft %ld \n", this->query_context.repeat, this->query_context.steps, expand_num__, this->query_context.frontiers->size());

    if ( this->query_context.frontiers->size() == 0 )
    {
        int frontiers_size_after = 0;
        int frontiers_size_delta = 0;
        int time_macros = 0;
        int global_best_dis = (int)(this->query_context.global_best_dis);
        int cost_limit = (int)(this->query_context.cost_limit);
        // int reward = -1*(this->query_context.cumulated_time)/1000; // ms
        int reward = 0;

        return std::vector<int>{1,
            time_macros,
            cost_limit,
            (int)(this->query_context.minimal_distance),
            frontiers_size_after, // sign for end
            frontiers_size_delta,
            global_best_dis,
            global_best_dis,
            global_best_dis,
            cost_limit,
            0,
            0,
            reward,
        };

    }
        

    
    TimeMeasurer t_step;
    t_step.resume();

   
    int min_label_per_thd = 10;
    // int min_label_per_thd = 2;
    float expand_ratio = 0.03;
    int used_thread;
    int expand_num;
    int interval;
    int frontiers_size_before = this->query_context.frontiers->size();

    

    if (frontiers_size_before < 100)
    {
        // expand_num = frontiers_size_before;
        expand_num = expand_num__;
        expand_num = min(frontiers_size_before, expand_num);
        
        // used_thread = 1;
        // used_thread = int(expand_num / min_label_per_thd);
        // used_thread = max(1, used_thread);

        // used_thread = 1;

        // expand_num = this->query_context.frontiers->size();
        // interval = 0;
        // interval = (int)(expand_num/used_thread);
    }
    else
    {   
        // expand_num = (int)(expand_ratio* this->query_context.frontiers->size());
        // expand_num = min(500, expand_num);
        // expand_num = max(100, expand_num);
        
        expand_num = expand_num__; // RL SETTING
        expand_num = min(frontiers_size_before, expand_num);

        // used_thread = 1;

        
    }
    // actually used thread nums
    used_thread = int(expand_num / min_label_per_thd);
    used_thread = min(thread_pool_size, used_thread); 
    used_thread = max(1, used_thread);

    bool single_thread_run = true;
    if (single_thread_run)
        used_thread = 1; // single thread setting



    interval = (int)(expand_num/used_thread);

    
    // t_step.pause();
    // printf("(before expand) ft size: %ld, expand num: %d, used thread: %d, workload p_thd: %d\n", this->query_context.frontiers->size(), expand_num, used_thread, interval);
    // t_step.resume();

    // multi-thread running
    std::vector<std::vector<Label_t> *> stores(used_thread);
    std::vector<std::future<  std::tuple<std::map<int, std::vector<Label_t>> *, std::vector<Label_t> > >> results;
    std::vector< std::tuple<std::map<int, std::vector<Label_t>> *, std::vector<Label_t> > >  results_get(used_thread);
    for (int i=0; i<used_thread-1; i++) // pop labels
    {
        stores[i] = pop_labels<Label_t>(this->query_context.frontiers, interval);
        results.emplace_back(thd_pool.enqueue(expand_worker<Label_t, Edge_t>, this, stores[i], this->query_context.dominace_list, this->query_context.u, this->query_context.v, this->query_context.cost_limit));

    }
    stores[used_thread-1] = pop_labels<Label_t>(this->query_context.frontiers, expand_num-interval*(used_thread-1));
    results.emplace_back(thd_pool.enqueue(expand_worker<Label_t, Edge_t>, this, stores[used_thread-1], this->query_context.dominace_list, this->query_context.u, this->query_context.v, this->query_context.cost_limit));
    
    // debug print
    // float poped_best_dis;
    // if ( (stores[used_thread-1])->size() > 0 )
        // poped_best_dis = (stores[used_thread-1])->back().distance_estimated;
    // else poped_best_dis = -1;
    // printf("current best dis: %.0f, expanded last estimated_dis: %.0f\n", this->global_best_dis, poped_best_dis);
    ////////////////////////////////


    
    // for (int i=0; i<used_thread-1; i++)
    // {
    //     results.emplace_back(thd_pool.enqueue(expand_worker<Label_t, Edge_t>, this, stores[i], this->query_context.dominace_list, this->query_context.u, this->query_context.v, this->query_context.cost_limit));
    // }
    // results.emplace_back(thd_pool.enqueue(expand_worker<Label_t, Edge_t>, this, stores[used_thread-1], this->query_context.dominace_list, this->query_context.u, this->query_context.v, this->query_context.cost_limit));
    
    // sync
    for (int i=0; i<used_thread; i++)
    {
        results_get[i] = results[i].get();
    }

    // collect multi-thread results
    std::map<int, std::vector<Label_t>> non_dominate_labels;
    std::map<int, std::vector<Label_t>> * next_round;
    std::vector< std::map<int, std::vector<Label_t>> * > next_round_for_free;
    std::vector<Label_t> sink_labels;
    int average_next_round_size = 0;
    for (int i=0; i<used_thread; i++)
    {
        std::tie(next_round, sink_labels) = results_get[i];
        next_round_for_free.push_back(next_round);
        for (auto it=next_round->begin(); it!= next_round->end(); it++)
            for (auto it_label=it->second.begin(); it_label!=it->second.end(); it_label++)
            {
                if (it_label->valid)
                {
                    add_new_frontier(&non_dominate_labels, *it_label);
                    
                }
                    
            }
        
        for (auto label: sink_labels)
        {
            // dominace_list[sink].push_back(label); // // push sink_labels to dominace_list[sink]
            add_new_frontier(this->query_context.dominace_list, label);
        }
    }
    // while (frontiers.size()!=0)
    // {
    //     auto label = frontiers.top();
    //     frontiers.pop();
    //     add_new_frontier(non_dominate_labels, label, true);
    // }
    for (auto it=non_dominate_labels.begin(); it!= non_dominate_labels.end(); it++)
        for (auto it_label=it->second.begin(); it_label!=it->second.end(); it_label++)
        {   
            if (it_label->valid)
            {
                this->query_context.frontiers->push(*it_label);
                add_new_frontier(this->query_context.dominace_list, *it_label);  // push new labels to dominace_list
                average_next_round_size += 1;
            }
                
        }
    
    // debug, save each iteration's expanded label node ids
    // this->each_iter_expanded_labels_of << this->query_context.frontiers->top().vid << endl;
    // debug


    // free storage
    free_heap<Label_t>(stores);
    free_heap<Label_t>(next_round_for_free);
    t_step.pause();
    this->query_context.cumulated_time += (int)(t_step.micros_count_);


    // setting observations and rewards
    int frontiers_size_after = this->query_context.frontiers->size();
    int frontiers_size_delta = frontiers_size_after - frontiers_size_before;
    int time_macros = (int)(t_step.micros_count_);
    int global_best_dis = (int)(this->query_context.global_best_dis);
    int cost_limit = (int)(this->query_context.cost_limit);
    int top_label_dis = (int)(this->query_context.frontiers->top().d);
    int top_label_dis_estimated = (int)(this->query_context.frontiers->top().distance_estimated);
    int top_label_cost = (int)(this->query_context.frontiers->top().delay);
    int top_label_cost_rest = (int)(cost_limit - top_label_cost);
    // int reward = -1*(this->query_context.cumulated_time + 500*this->query_context.steps)/1000; // ms
    // average_next_round_size /= used_thread;
    // int reward = -1*time_macros;
    int reward = -1*average_next_round_size;
    
    return std::vector<int>{0, // done flag
        time_macros,  // time of this iteration
        cost_limit,  // (fixed) cost limit
        (int)(this->query_context.minimal_distance),  // (fixed) shortest distance of this query
        frontiers_size_after,  // current frontier size
        frontiers_size_delta,  // frontier size change
        global_best_dis,  // current global best distance
        top_label_dis_estimated,  // label.d + mininal_distance
        top_label_dis,  // label.d
        top_label_cost,  // label.delay
        global_best_dis - top_label_dis_estimated,  // current global best distance - (label.d + mininal_distance)
        top_label_cost_rest,  // cost limit - label.delay
        reward,  // reward, time of this iteration * -1
    };



}

template<typename Label_t, typename Edge_t>
void CSPP<Label_t, Edge_t>::solve_one_query(int u, int v, float cost_limit) {
    // initialize
    printf("u %d v %d  cost_limit %.0f\n", u, v, cost_limit);


    auto res = this->reset(u, v, cost_limit);
    auto state_tensor = torch::tensor({0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0});
    at::Tensor dqn_output;

    
    // iterating
    
    int iter = 0;
    long long int query_us = 0;
    long long int pred_us = 0;

    // int next_expand_num = 100; // default
    // int next_expand_num = 1024;
    int next_expand_num = this->default_expand_num;


    TimeMeasurer t_overall;
    t_overall.resume();
    while ( true )
    {
        iter++;    
        // for debug
        int position;
        printf("iter: %d, current global best: %.0f, frontier top estimated_dis: %.0f\n", iter, this->query_context.global_best_dis, this->query_context.frontiers->top().distance_estimated);

        // if (this->query_context.csp_truth_file_exist && this->truth_position)
        // {
        //     position = check_frontier_first_csp_label<Label_t>(&set_labels, this->query_context.frontiers);
        //     printf("first position (csp labels): %d\n", position);
        // }
        ////////////////////////////////
        // int expand_num = (rand()%7 +1)*100;
        // auto step_state = one_iteration(expand_num);

        auto step_state = this->one_iteration(next_expand_num);
        /*
        step_state:
        0: done
        1: time_macros (us, 10^-6 s), time elapsed in this step/iteration
        ...: observations
        -1: reward
        */
        bool terminate = step_state[0];
        int step_us = step_state[1];

        /* dqn control */
        if (this->rl_expand_control)
        {
        if (iter % 4 ==0) {
        TimeMeasurer t_pred;
        t_pred.resume();
        state_tensor[0] = step_state[1];
        state_tensor[1] = step_state[2];
        state_tensor[2] = step_state[3];
        state_tensor[3] = step_state[4];
        state_tensor[4] = step_state[5];
        state_tensor[5] = step_state[6];
        state_tensor[6] = step_state[7];
        state_tensor[7] = step_state[8];
        dqn_output = this->dqn->forward(state_tensor);
        int argmax = 0;
        float argmax_value = *(dqn_output[0].data_ptr<float>());
        for (int i = 0; i < 7; i++){
            if ( *(dqn_output[i].data_ptr<float>()) > argmax_value){
                argmax_value = *(dqn_output[i].data_ptr<float>());
                argmax = i;
            }
        }
        
        // next_expand_num = (argmax+1) * 100; // 100 interval actions, 100, ..., 700
        next_expand_num = 1<<(argmax+4); // exp interval actions, 16, 32, ..., 1024
        t_pred.pause();
        pred_us += t_pred.micros_count_;
        }
        }
        /* dqn control */



        query_us += step_us;

        if (terminate)
            break;

        cout << "(after expand) ft size: " << this->query_context.frontiers->size() << endl;
        printf("iteration time (micros): %d\n", step_us );
        cout << endl;

        ////////// debug, save each iteration's expanded label node ids
        // should be commented when testing times
        // if (taskid==19) 
        // {
        // for (auto it=non_dominate_labels.begin(); it!= non_dominate_labels.end(); it++)
        //     for (auto it_label=it->second.begin(); it_label!=it->second.end(); it_label++)
        //     {   
        //         if (it_label->valid)
        //         {
        //             this->each_iter_expanded_labels_of << it_label->vid << " ";
        //         }
                    
        //     }
        // this->each_iter_expanded_labels_of << endl;

        // this->each_iter_frontier_size_of << frontiers.size() << endl;        
        // }
        //////////

        // debug, check dominace_list
        // for (auto label : (*(this->query_context.dominace_list))[this->query_context.v] )
        // {
        //     if (label.vid != this->query_context.v)
        //     {
        //         exit(0);
        //     }
        // }
        

        
    }
    t_overall.pause();

    query_us += pred_us;

    printf("==: %lld ms (pred %lld ms)\n", query_us/1000, pred_us/1000);
    
    // this->execution_time.push_back(query_us); // us
    this->execution_time.push_back(t_overall.micros_count_); // us
    
    // for (auto label : (*(this->query_context.dominace_list))[this->query_context.v] )
    //     {
    //         if (label.vid != this->query_context.v)
    //         {
    //             exit(0);
    //         }
    //     }


    // print csp length
    auto sink_labels = (*(this->query_context.dominace_list))[this->query_context.v];
    printf("%ld labels on sink\n", sink_labels.size());
    // printf("before sort\n");
    // print_csp(&sink_labels);
    std::stable_sort(sink_labels.begin(), sink_labels.end(), max_distance<Label_t>);
    printf("after sort\n");
    print_csp(&sink_labels);
    // printf("1\n");

    // save sink labels
    if (prune_type == 1) 
        save_labels_on_sink<Label_t, Edge_t>(this->query_context.u, this->query_context.v, (*(this->query_context.dominace_list))[this->query_context.v], this); // save all non-dominate labels on sink only under the prune_type 1
    
    // save_csp_path, should be commented when comparing performance
    Label_t sink_best = sink_labels.back(); // has been sorted
    save_csp_path<Label_t, Edge_t>(this->query_context.u, this->query_context.v, sink_best, this->query_context.dominace_list, this);


    
}

template<typename Label_t, typename Edge_t>
int CSPP<Label_t, Edge_t>::RUN_CSPP_CPU_with_priority_thread() {
    c10::InferenceMode guard(true); // inference mode

    for (int taskid = 0; taskid < this->ntask; taskid++) {
        //    std::cout << C_l[i].delay_L << std::endl;
        // std::cout << "================== task " << i << std::endl;
        printf("================== task %d\n", taskid);

        int source = this->init_state[taskid].vid;
        int sink = sink_l[taskid];
        float cost_limit = C_l[taskid].delay_L;

        this->solve_one_query(source, sink, cost_limit);

    }

    // total_time.print_ms("Total Time is");
    long long total_time = 0;
    for (auto time : this->execution_time)
        total_time += time;
    
    printf("Expanded label nums: %d\n", this->total_expanded_labels);
    printf("Prediction nums (not pruned): %d\n", this->prediction_nums);
    printf("Pruned nums: %d\n", this->total_expanded_labels - this->prediction_nums);
    printf("Total Time is %.3f ms\n", (float)total_time/1000);
    printf("Per time: ");
    for (auto time : this->execution_time)
        printf("%.3f, ", (float)time/1000 );
    printf("\n");

    return 0;

}




 
#endif
