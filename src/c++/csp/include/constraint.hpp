#ifndef _csp_hpp_
#define _csp_hpp_
#include "label.hpp"

template<typename Label_t>
class constraint {
 public:
  constraint() {
    return;
  }
  virtual void set_constraints() {
    return;
  }

};

#if CSP == 1
template<typename Label_t>
class c_constraint {
 public    :
  float delay_L;
 public    :
  c_constraint() {
    return;
  }
  c_constraint(float d) : delay_L(d) {
    return;
  }
  void set_constraints(int d) {
    delay_L = d;

    return;
  }

  bool pass_constraints_check_cpu(const Label_t &l) {
    return !(l.delay > delay_L);
  }
};

#endif

#endif
