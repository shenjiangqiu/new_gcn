//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_SYSTEM_H
#define GCN_SIM_SYSTEM_H

#include <memory>

#include "Aggregator.h"
#include "SystolicArray.h"
#include "buffer.h"
#include "memory_interface.h"

class System {
public:
  System(int inputBufferSize, int edgeBufferSize, int aggBufferSize,
         int outputBufferSize, int aggTotalCores, int systolic_rows,
         int systolic_cols, std::string graphName, std::vector<int> node_size,
         std::string dram_config_name);

  void cycle();
  void run();

private:
  std::shared_ptr<ReadBuffer> input_buffer;
  std::shared_ptr<ReadBuffer> edge_buffer;
  std::shared_ptr<WriteBuffer> output_buffer;
  std::shared_ptr<Aggregator_buffer> agg_buffer;
  std::shared_ptr<memory_interface> m_mem;
  std::shared_ptr<Aggregator> m_aggregator;
  std::shared_ptr<SystolicArray> m_systolic_array;
  std::shared_ptr<Slide_window_set> m_slide_window_set;

  std::string graph_name;

  std::shared_ptr<Graph> m_graph;

  int input_buffer_size;
  int edge_buffer_size;
  int agg_buffer_size;
  int output_buffer_size;

  int agg_total_cores;

  bool finished{false};
  std::shared_ptr<slide_window_set_iterator> current_iter;
  std::shared_ptr<slide_window_set_iterator> prev_iter;
};

#endif // GCN_SIM_SYSTEM_H
