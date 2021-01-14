//
// Created by sjq on 1/8/21.
//

#ifndef GCN_SIM_GLOBALS_H
#define GCN_SIM_GLOBALS_H

class globals {
public:
  unsigned long long cycle = 0;

  // this will be set by the output buffer when the last col's output is write
  // down to the memory
  bool finished = false;
};
extern globals global_definitions;

#endif // GCN_SIM_GLOBALS_H
