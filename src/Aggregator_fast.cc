//
// Created by sjq on 1/6/21.
//

#include "Aggregator_fast.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>

#include "debug_helper.h"
#include <algorithm>
#include <sparseVector.h>

namespace fast_sched {
Aggregator_fast::Aggregator_fast(int totalCores) : total_cores(totalCores) {}

void Aggregator_fast::cycle() {
  if (working) {
    // if (remaining_cycle >= 100000) {
    //   GCN_INFO_S("remaining cycle too large!!");
    // }
    assert(remaining_cycle > 0);
    // if (remaining_cycle == 0) {
    //   throw std::runtime_error("the cycle should not be zero");
    // }
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

void Aggregator_fast::add_task(const shared_ptr<Req> &req, unsigned /*node dim*/,
                               sparseVector &m_vec) {
  assert(working == false);
  assert(req->items_cnt != 0);
  auto total_edges = req->items_cnt;
  total_operations += total_edges;
  total_rounds++;
  //auto rounds = (total_edges * node_dim + total_cores - 1) / total_cores;
  // read dram latency;
  //auto per_round_memory_fetch_time = 1;
  // for each round, read the data, and use 1 cycle to process.

  // **** old code ****
  // auto total_time = rounds * (per_round_memory_fetch_time);

  // global_definitions.do_aggregate += total_time;
  // global_definitions.total_aggregate_op += total_edges * node_dim;
  // global_definitions.total_input_windows++;
  // global_definitions.total_edges += total_edges;

  // **** new code ****
  // get the tasks from the req
  auto tasks = req->edges;

  // build the parameters
  auto p_task = std::vector<std::vector<unsigned>>();
  auto p_output = std::vector<unsigned>();
  for (auto i = 0u; i < tasks.size(); i++) {
    p_task.push_back(tasks[i].second);
    p_output.push_back(tasks[i].first);
  }

  // TODO : where is the layers
  remaining_cycle = m_vec.getAddCycles(p_task, p_output, req->current_layer);
  // divide the cycles by num of cores
  remaining_cycle = (remaining_cycle + total_cores - 1) / total_cores;

  // statistics
  global_definitions.sparse_agg_cycles+=remaining_cycle;
  // debuf info
  static int times = 0;
  if (times % 100 == 0) {
    GCN_INFO("add task {} times,remaining_cycle {}, num_tasks: {} ,current "
             "layer: {}",
             times, remaining_cycle, tasks.size(), req->current_layer);
  }
  times++;

  if (remaining_cycle >= 1000000) {
    throw std::runtime_error("remaining cycle too large!");
  }
  working = true;
}
} // namespace fast_sched