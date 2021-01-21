//
// Created by sjq on 1/8/21.
//

#ifndef GCN_SIM_GLOBALS_H
#define GCN_SIM_GLOBALS_H
#include "Model.h"
#include "utils/Options.h"
#include "vector"
#include "map"
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

  std::vector<unsigned long long> finished_time_stamp;
  std::map<std::string, Model> m_models{
      {"gcn", Model("gcn", {128}, true, false, 0)},
      {"gsc", Model("gsc", {128}, true, true, 25)},
      {"gin", Model("gin", {128, 128}, true, false, 0)}};
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
extern Minisat::IntOption ignore_neighbor;
extern Minisat::IntOption ignore_self;
} // namespace config
#endif // GCN_SIM_GLOBALS_H
