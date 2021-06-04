//
// Created by sjq on 1/6/21.
//

#include "Aggregator_fast.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>

#include "debug_helper.h"
#include <algorithm>

namespace fast_sched {
Aggregator_fast::Aggregator_fast(int totalCores) : total_cores(totalCores) {}

void Aggregator_fast::cycle() {
  if (working) {
    remaining_cycle--;
    if (remaining_cycle == 0) {
      working = false;
    }
  }
}
/*

void Aggregator_fast::add_task(std::shared_ptr<dense_window> window) {
  assert(empty and !working and remaining_cycles == 0);
  assert(!current_sliding_window);
  current_sliding_window = std::move(window);
  empty = false;
  finished = false;
  GCN_DEBUG("aggregator add aggregator task. x: {} y: {},cycle: {}",
                current_sliding_window->getX(), current_sliding_window->getY(),
                global_definitions.cycle);
}
*/

// 123

void Aggregator_fast::add_task(const shared_ptr<Req> &req, unsigned node_size) {
  assert(working == false);
  auto total_edges = req->items_cnt;
  total_operations+=total_edges;
  total_rounds++;
  auto rounds = (total_edges * node_size + total_cores - 1) / total_cores;
  // read dram latency;
  auto per_round_memory_fetch_time = (total_cores * 4 + 31) / 32;
  // for each round, read the data, and use 1 cycle to process.
  auto total_time = rounds * (per_round_memory_fetch_time + 1);

  remaining_cycle = total_time;
  if(remaining_cycle>=100000){
    GCN_INFO("remaining_cycle too large:{}",remaining_cycle);
  }
  working = true;
}
} // namespace fast_sched