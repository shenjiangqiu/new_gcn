//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_SYSTEM_H
#define GCN_SIM_SYSTEM_H

#include <memory>

#include "Aggregator.h"
#include "Model.h"
#include "SystolicArray.h"
#include "buffer.h"
#include "memory_interface.h"
#include <Slide_window.h>
#include <sparseVector.h>
#include <utils/average_number.hh>
class System {
public:
  System(int inputBufferSize, int edgeBufferSize, int aggBufferSize,
         int outputBufferSize, int aggTotalCores, int systolic_rows,
         int systolic_cols, std::shared_ptr<Graph> graph,
         std::vector<int> node_dim, const std::string &dram_config_name,
         std::shared_ptr<Model> mModel,
         std::shared_ptr<sparseVector> m_vec, bool enable_sparse);
  void print() const;
  void cycle();
  void run();

  std::shared_ptr<dense_window_set> get_sliding_window() {
    return m_slide_window_set;
  }

private:
  bool enable_sparse;
  std::shared_ptr<sparseVector> m_vec;
  std::vector<average_number> average_window_size;
  std::shared_ptr<Graph> m_graph;

  int input_buffer_size;
  int edge_buffer_size;
  int agg_buffer_size;
  int output_buffer_size;

  int agg_total_cores;
  std::shared_ptr<dense_window_set> m_slide_window_set;
  std::shared_ptr<InputBuffer> input_buffer;
  std::shared_ptr<EdgeBuffer> edge_buffer;
  std::shared_ptr<WriteBuffer> output_buffer;
  std::shared_ptr<Aggregator_buffer> agg_buffer;
  std::shared_ptr<memory_interface> m_mem;
  std::shared_ptr<Aggregator> m_aggregator;
  std::shared_ptr<SystolicArray> m_systolic_array;

  window_iter current_iter;
  window_iter prev_iter;
  std::shared_ptr<Model> m_model;
  double current_system_time = 0;
  double current_dram_time = 0;
  double cpu_gap = 0;
  double dram_gap = 0;
};

#endif // GCN_SIM_SYSTEM_H
