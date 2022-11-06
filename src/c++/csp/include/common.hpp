#ifndef _common_
#define _common_

#include<tuple>

template<typename Label_t>
struct label_comp_
{
  bool operator()(Label_t a, Label_t b){
    // delay is cost

    // if (a.d == b.d) return a.delay > b.delay;
    // else  return a.d > b.d;
    

    // if ( a.priority == b.priority ) return a.delay > b.delay;
    // else return a.priority > b.priority;


    // if ( a.priority == b.priority ) return a.distance_estimated > b.distance_estimated;
    // else return a.priority < b.priority;


    if (a.distance_estimated == b.distance_estimated)
        return a.delay > b.delay;
    else
        return a.distance_estimated > b.distance_estimated;


  }

};

// template<typename Label_t>
// void test_print(std::map<int, std::vector<c_label>>& d, int i) {
//     if (d.find(i) == d.end()) {
//         std::cout << "No item.\n" << std::endl;
//         return;
//     }
//     printf("label list size: %ld\n", d[i].size());
//     for (auto label : d[i]) {
//         printf("vid: %d, d: %.2f, delay: %.2f\n", label.vid, label.d, label.delay);  
//     }
// }



template<typename Label_t>
struct label_comp_get_min_distance
{
  // The value returned indicates whether the element passed as first argument is considered less than the second.
  bool operator()(Label_t a, Label_t b){
    return a.d >= b.d;
  }
};

template<typename Label_t>
bool max_distance(Label_t a, Label_t b)
{
    // The value returned indicates whether the element passed as first argument is considered less than the second.
    return a.d >= b.d;
};

template<typename Label_t>
struct label_comp_get_max_cost
{
  // The value returned indicates whether the element passed as first argument is considered less than the second.
  bool operator()(Label_t a, Label_t b){
    return a.delay <= b.delay;
  }
};

template<typename Label_t>
bool min_distance(Label_t a, Label_t b) {
    return a.d < b.d;
}


struct set_labels_comp
{
    bool operator()(std::tuple<int, int, int> a, std::tuple<int, int, int> b)
    {
        return std::get<1>(a) < std::get<1>(b);
    }
    // comp is an object of this type and a and b are key values, shall return true if a is considered to go before b
};

#endif