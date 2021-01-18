//
// Created by sjq on 1/6/21.
//

#include "Aggregator.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <utility>

Aggregator::Aggregator(const shared_ptr<InputBuffer> &inputBuffer,
                       const shared_ptr<EdgeBuffer> &edgeBuffer,
                       const shared_ptr<Aggregator_buffer> &aggBuffer,
                       int totalCores)
    : input_buffer(inputBuffer), edge_buffer(edgeBuffer), agg_buffer(aggBuffer),
      total_cores(totalCores) {}

void Aggregator::cycle() {

  if (working) {
    // current running one task
    assert(remaining_cycles and current_sliding_window);
    remaining_cycles--;
    if (remaining_cycles == 0) {
      working = false;
      spdlog::debug("aggregator finished run task. x: {} y: {},cycle: {}",
                    current_sliding_window->getX(),
                    current_sliding_window->getY(), global_definitions.cycle);

      input_buffer->finished_current();
      // decide if finished edge buffer
      if (current_sliding_window->isTheFinalRow()) {
        edge_buffer->finished_current();
        agg_buffer->finish_write();
      }
    }
  }
  if (!working) {
    // have task but not running
    assert(remaining_cycles == 0);
    // if this is the new  col, the agg_buffer should be emtpy
    // have task and finshed, need waite the agg buffer move it to rad buffer
    if (!agg_buffer->isWriteEmpty() and agg_buffer->isWriteReady()) {
      return;
    }
    if (input_buffer->isCurrentReady() and edge_buffer->isCurrentReady() and
        (agg_buffer->isWriteEmpty() or
         (current_sliding_window and agg_buffer->getWriteWindow()->getX() ==
                                         current_sliding_window->getX()))) {
      current_sliding_window =
          std::make_shared<Slide_window>(*(input_buffer->getMCurrentIter()));
      remaining_cycles = calculate_remaining_cycle();
      working = true;

      if (current_sliding_window->isTheFirstRow()) {
        agg_buffer->add_new_task(current_sliding_window);
      }
      spdlog::debug("aggregator start to run task x: {} y: {} ,cycle: {}",
                    current_sliding_window->getX(),
                    current_sliding_window->getY(), global_definitions.cycle);
    }
  }
}
/*

void Aggregator::add_task(std::shared_ptr<Slide_window> window) {
  assert(empty and !working and remaining_cycles == 0);
  assert(!current_sliding_window);
  current_sliding_window = std::move(window);
  empty = false;
  finished = false;
  spdlog::debug("aggregator add aggregator task. x: {} y: {},cycle: {}",
                current_sliding_window->getX(), current_sliding_window->getY(),
                global_definitions.cycle);
}
*/

int Aggregator::calculate_remaining_cycle() {
  assert(current_sliding_window);
  auto total_nodes = current_sliding_window->getNumNodesInWindow();
  auto node_size = current_sliding_window
                       ->getCurrentNodeSize(); // num features in one node//not
                                               // the bytes in one nodes
  auto total_elements = total_nodes * node_size;
  spdlog::debug(
      "aggregator culculate window cycles. x: {} y: {}, result: {} ,cycle: {}",
      current_sliding_window->getX(), current_sliding_window->getY(),
      (total_elements + total_cores - 1) / total_cores,
      global_definitions.cycle);

  auto cycle = (total_elements + total_cores - 1) / total_cores;
  // read dram latency;
  auto per_cycle_memory_fetch_time = (total_cores * 4 + 63) / 64;
  auto total_read_memory_time = cycle * per_cycle_memory_fetch_time;
  cycle += total_read_memory_time;
  return cycle;
}
