//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_SYSTEM_fast_H
#define GCN_SIM_SYSTEM_fast_H

#include <memory>

#include "Aggregator_fast.h"
#include "Model.h"
#include "SystolicArray_fast.h"
#include "buffer_fast.h"
#include "controller.h"
#include "memory_interface.h"
namespace fast_sched {
class System {
public:
  System(int inputBuffer_fastSize, int edgeBufferSize, int aggBufferSize,
         int outputBufferSize, int aggTotalCores, int systolic_rows,
         int systolic_cols, std::shared_ptr<Graph> graph,
         std::vector<int> node_dim, const std::string &dram_config_name,
         std::shared_ptr<Model> mModel);
  void print() const;
  void cycle();
  void run();

private:
  std::shared_ptr<Graph> m_graph;

  int input_buffer_size;
  int edge_buffer_size;
  int agg_buffer_size;
  int output_buffer_size;

  int agg_total_cores;
  std::shared_ptr<InputBuffer> input_buffer;
  std::shared_ptr<memory_interface> m_mem;
  std::shared_ptr<Aggregator_fast> m_aggregator;
  std::shared_ptr<controller> m_controller;
  std::shared_ptr<Model> m_model;
  double current_system_time = 0;
  double current_dram_time = 0;
  double cpu_gap = 0;
  double dram_gap = 0;
};
} // namespace fast_sched
#endif // GCN_SIM_SYSTEM_H
