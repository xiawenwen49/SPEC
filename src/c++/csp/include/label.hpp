#ifndef _label_hpp_
#define _label_hpp_

// #include <cuda_runtime.h>
#include "edge.hpp"
#include "io.hpp"

#ifndef CSP
#define CSP 1
#endif

// __device__ float *A_dd, *B_dd, *Theta_dd;

// __global__ void assign(float *A, float *B, float *Theta) {
//   A_dd = A;
//   B_dd = B;
//   Theta_dd = Theta;
// }

// class label {
//  public:
//   int vid;
//  public:
//   __device__ label() {
//     return;
//   }

//   __device__ virtual int get_vid() {
//     return vid;
//   }

//   __device__ label
//   expand(edge e) {
//     return label();
//   }

//   __device__ static bool pass_constraints_check() {
//     return false;
//   }
//   __device__ static bool dominate(label a, label b) {
//     return false;
//   }

//   ~label() {
//   }
// };


#if CSP == 1
class c_label {
 public    :
  int vid;
  float d; 
  float delay; // cost
  float distance_estimated=0;
  int priority=0;
  int hop = 0;
  bool valid=true;
  int father=-1;

 public    :
  __device__ c_label() {
    return;
  }

  c_label(int vid) : vid(vid) {
    d = 0; // distance
    delay = 0; // cost
    
  }


  c_label(int vid, float dis, float cost) : vid(vid), d(dis), delay(cost) {
    return;
  }
  c_label(int vid, float dis, float cost, int hop) : vid(vid), d(dis), delay(cost), hop(hop) {
    return;
  }
  c_label(int vid, float dis, float cost, float dis_estimated, int father, int hop) : vid(vid), d(dis), delay(cost), distance_estimated(dis_estimated), father(father), hop(hop) {
    return;
  }


  __device__ static bool equal(c_label *a, c_label *b) {
    return a->vid == b->vid && a->d == b->d && a->delay == b->delay;
  }

  __device__ static void copy(c_label *a, c_label *b) {
    a->vid = b->vid;
    a->d = b->d;
    a->delay = b->delay;
  }

  __device__ int get_vid() {
    return vid;
  }

  template<typename Edge_t>
  __device__ void expand(int next_v, Edge_t e, c_label *son) {
    son->vid = next_v;
    son->d = this->d + e.len;
    son->delay = this->delay + e.delay;
    son->hop = this->hop + 1;
  }

  template<typename Edge_t>
  void expand_cpu(int next_v, Edge_t e, c_label *son) {
    son->vid = next_v;
    son->d = this->d + e.len;
    son->delay = this->delay + e.delay;
    son->hop = this->hop + 1;
    son->father = this->vid;
  }

  static bool dominate_cpu(c_label a, c_label b) {
    return a.d <= b.d && a.delay <= b.delay;
  }
    void replace(c_label a) {
        this->d = a.d;
        this->delay = a.delay;
    }

};
#endif

#endif