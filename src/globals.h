//
// Created by sjq on 1/8/21.
//

#ifndef GCN_SIM_GLOBALS_H
#define GCN_SIM_GLOBALS_H
#include "Model.h"
#include "map"
#include "utils/Options.h"
#include "vector"
#define add_config_uint64() { \
                              \
  }
#define add_config_ull() {}
#define add_config_ull_v() {}
#define add_config_uint64_v() {}

class globals {
public:

  uint64_t cycle = 0;

  uint64_t edgeBuffer_idle_cycles = 0;
  uint64_t inputBuffer_idle_cycles = 0;

  // this will be set by the output buffer when the last col's output is write
  // down to the memory
  bool finished = false;
  bool concate = false;
  bool initialResidual = false;

  using ull = unsigned long long;
  ull total_read_traffic = 0;
  ull total_write_traffic = 0;
  ull total_read_traffic_input = 0;
  ull total_read_traffic_edge = 0;

  uint64_t total_waiting_input = 0;
  uint64_t total_waiting_edge = 0;
  uint64_t total_waiting_agg_write = 0;
  uint64_t do_aggregate = 0;
  uint64_t total_aggregate_op = 0;
  uint64_t total_edges = 0;
  uint64_t total_input_windows = 0;
  std::vector<uint64_t> layer_input_windows;
  std::vector<uint64_t> layer_edges;
  std::vector<uint64_t> layer_input_vertics;
  std::vector<uint64_t> layer_do_aggregate;
  std::vector<uint64_t> layer_aggregate_op;
  std::vector<uint64_t> layer_do_systolic;
  std::vector<uint64_t> layer_wait_input;
  std::vector<float> layer_window_avg_agg;
  std::vector<float> layer_window_avg_input;
  uint64_t do_systolic = 0;
  uint64_t total_waiting_agg_read = 0;
  uint64_t total_waiting_out = 0;
  uint64_t total_read_input_latency = 0;
  uint64_t total_read_input_vertices_cnt = 0;
  uint64_t total_read_input_times = 0;
  uint64_t total_read_input_len = 0;
  uint64_t total_read_edge_latency = 0;
  uint64_t total_read_edge_times = 0;
  uint64_t total_read_edge_len = 0;
  uint64_t total_mac_in_systolic_array = 0;
  uint64_t total_read_input_traffic = 0;
  uint64_t total_read_edge_traffic = 0;
  // this cycle, inputbuffer idle
  uint64_t total_input_buffer_idle = 0;
  // this cycle, edge idle
  uint64_t total_edge_buffer_idle = 0;
  // this cycle, all buffer idle, the dram have no work to do!
  uint64_t all_buffer_idle = 0;

  std::vector<uint64_t> finished_time_stamp;

  std::map<std::string, Model> m_models{
      {"gcn", Model("gcn", {16, 16}, false, false, 0)},
      {"gsc", Model("gsc", {512, 512}, false, true, 25)},
      {"gin", Model("gin", {64, 64, 64, 64, 64}, false, false, 0)}};

  std::map<std::string, uint64_t> config_uint64;
  std::map<std::string, ull> config_ull;

  std::map<std::string, std::vector<uint64_t>> config_uint64_v;
  std::map<std::string, std::vector<ull>> config_ull_v;
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
extern Minisat::StringOption mem_sim;
extern Minisat::IntOption enable_feature_sparsity;
extern Minisat::DoubleOption feature_sparse_rate0;
extern Minisat::DoubleOption feature_sparse_rate1;
extern Minisat::DoubleOption feature_sparse_rate2;
extern Minisat::BoolOption enable_valid_node_only;

extern Minisat::BoolOption enable_dense_window;
} // namespace config
#endif // GCN_SIM_GLOBALS_H
