//
// Created by sjq on 1/12/21.
//

#ifndef GCN_SIM_HELPER_UTILS_H
#define GCN_SIM_HELPER_UTILS_H
#include "Slide_window.h"
class helper_utils {
public:
  static slide_window_set_iterator next(slide_window_set_iterator i) {
    return ++i;
  }
};

#endif // GCN_SIM_HELPER_UTILS_H
