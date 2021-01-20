//
// Created by sjq on 1/8/21.
//

#ifndef GCN_SIM_GLOBALS_H
#define GCN_SIM_GLOBALS_H
#include "utils/Options.h"

class globals {
public:
  unsigned long long cycle = 0;

  // this will be set by the output buffer when the last col's output is write
  // down to the memory
  bool finished = false;
  bool concate = false;

  unsigned long long total_waiting_input = 0;
  unsigned long long total_waiting_edge = 0;
  unsigned long long total_waiting_agg_write = 0;
  unsigned long long do_aggregate = 0;
  unsigned long long do_systolic = 0;
  unsigned long long total_waiting_agg_read = 0;
  unsigned long long total_waiting_out = 0;
  unsigned long long total_read_input_latency = 0;
  unsigned long long total_read_input_times = 0;
  unsigned long long total_read_edge_latency = 0;
  unsigned long long total_read_edge_times = 0;
};
extern globals global_definitions;

namespace config {

extern Minisat::IntOption inputSize;
extern Minisat::IntOption outputSize;
extern Minisat::IntOption edgeSize;
extern Minisat::IntOption aggSize;
extern Minisat::IntOption aggCores;
extern Minisat::IntOption systolic_rows;
extern Minisat::IntOption systolic_cols;
extern Minisat::StringOption graph_name;
extern Minisat::StringOption dram_name;
extern Minisat::BoolOption debug;
extern Minisat::StringOption model;
} // namespace config
#endif // GCN_SIM_GLOBALS_H
