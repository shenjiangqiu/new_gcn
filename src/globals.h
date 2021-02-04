//
// Created by sjq on 1/8/21.
//

#ifndef GCN_SIM_GLOBALS_H
#define GCN_SIM_GLOBALS_H
#include "Model.h"
#include "map"
#include "utils/Options.h"
#include "vector"
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
  unsigned long long total_mac_in_systolic_array = 0;
  unsigned long long total_read_input_traffic = 0;
  unsigned long long total_read_edge_traffic = 0;
  std::vector<unsigned long long> finished_time_stamp;
  std::map<std::string, Model> m_models{
      {"gcn", Model("gcn", {16, 16}, false, false, 0)},
      {"gsc", Model("gsc", {512, 512}, false, true, 25)},
      {"gin", Model("gin", {64, 64, 64, 64, 64}, false, false, 0)}};
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
extern Minisat::DoubleOption core_freq;
extern Minisat::DoubleOption dram_freq;
extern Minisat::BoolOption use_dramsim;
} // namespace config
#endif // GCN_SIM_GLOBALS_H
