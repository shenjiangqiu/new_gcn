//
// Created by sjq on 1/6/21.
//

#ifndef GCN_SIM_AGGREGATOR_H
#define GCN_SIM_AGGREGATOR_H

#include "buffer.h"

#include "sliding_window_dense.h"

#include <unordered_map>
#include <vector>

class Aggregator {
public:
  [[nodiscard]] bool isWorking() const { return working; }
  // TODO read buffer have latency
  void cycle();

  Aggregator(const shared_ptr<InputBuffer> &inputBuffer,
             const shared_ptr<EdgeBuffer> &edgeBuffer,
             const shared_ptr<Aggregator_buffer> &aggBuffer, int totalCores);

  ~Aggregator();

private:
  //
  std::shared_ptr<InputBuffer> input_buffer;
  std::shared_ptr<EdgeBuffer> edge_buffer;
  std::shared_ptr<Aggregator_buffer> agg_buffer;

  //get remaining cycle base on current_sliding_window
  int calculate_remaining_cycle();


  std::shared_ptr<sliding_window_interface> current_sliding_window;
  int total_cores;
  int cur_layer;

  using HistoCount = std::unordered_map<int, uint64_t>;
  HistoCount edgesInWindowHist, inputEffHist;

#define EDGE 0
#define EFF 1
  void updateEdgesHist(int flag, int edge_cnt);

  bool working{false};

  int remaining_cycles{0};
};

#endif // GCN_SIM_AGGREGATOR_H
