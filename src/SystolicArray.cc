//
// Created by sjq on 1/6/21.
//

#include "SystolicArray.h"
#include "globals.h"
#include "spdlog/spdlog.h"
SystolicArray::SystolicArray(int totalRows, int totalCols,
                             const shared_ptr<Aggregator_buffer> &aggBuffer,
                             const shared_ptr<WriteBuffer> &outputBuffer)
    : total_rows(totalRows), total_cols(totalCols),
      current_sliding_window(nullptr), agg_buffer(aggBuffer),
      output_buffer(outputBuffer) {}
int SystolicArray::cal_remaining_cycle() {

  assert(current_sliding_window);
  int total_cycles = 0;
  auto num_nodes = current_sliding_window->getXw();
  auto node_size =
      current_sliding_window->getCurrentNodeSize(); // num elements in one node;
  auto steps = (total_rows + num_nodes - 1) / num_nodes;
  auto elements_steps = (node_size + total_cols - 1) / total_cols;
  for (auto i = 0; i < steps - 1; i++) {
    for (auto j = 0; j < elements_steps - 1; j++) {
      total_cycles += (total_rows + total_cols);
    }

    total_cycles += total_rows + node_size - (elements_steps * total_cols);
  }
  total_cycles += num_nodes - (steps * total_rows) + node_size -
                  (elements_steps * total_cols);
  return total_cycles;
}
void SystolicArray::cycle() {
  if (empty and agg_buffer->isReadReady() and !agg_buffer->isReadBusy() and
      !agg_buffer->isReadEmpty() and output_buffer->isNextEmpty()) {
    assert(remaining_cycle == 0);
    empty = false;
    running = true;
    current_sliding_window = agg_buffer->getReadWindow();

    remaining_cycle = cal_remaining_cycle();
    
    //TODO need to set agg buffer and output buffer here
    spdlog::debug("start a new systolic task: window:{}, total_cycle:{}, "
                  "current cycle:{}",
                  *current_sliding_window, remaining_cycle,
                  global_definitions.cycle);
  }
  if (remaining_cycle != 0) {
    remaining_cycle--;
    if (remaining_cycle == 0) {
      spdlog::debug("end a systolic array, window:{}, current_cycle:{}",
                    *current_sliding_window, global_definitions.cycle);
      current_sliding_window = nullptr;

      agg_buffer->finish_read();
      //TODO need to update output buffer here
      empty = true;
      running = false;
      finished = true;
    }
  }
}
