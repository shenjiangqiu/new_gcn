//
// Created by sjq on 1/6/21.
//

#include "SystolicArray.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <memory>
SystolicArray::SystolicArray(int totalRows, int totalCols,
                             const shared_ptr<Aggregator_buffer> &aggBuffer,
                             const shared_ptr<WriteBuffer> &outputBuffer)
    : total_rows(totalRows), total_cols(totalCols),
      current_sliding_window(nullptr), agg_buffer(aggBuffer),
      output_buffer(outputBuffer) {}
int SystolicArray::cal_remaining_cycle() {
  // TODO: Add output latency, the weight is decided by output layer dimension.
  assert(current_sliding_window);
  int total_cycles = 0;
  auto num_nodes = current_sliding_window->getXw();
  auto node_size =
      current_sliding_window->getCurrentNodeSize(); // num elements in one node;
  // TODO: here we should get next level node size
  auto next_node_size = 128;
  if (global_definitions.concate) {
    node_size *= 2;
  }
  auto steps = (total_rows + num_nodes - 1) / num_nodes;
  auto elements_steps = (next_node_size + total_cols - 1) / total_cols;
  for (auto i = 0; i < steps - 1; i++) {
    for (auto j = 0; j < elements_steps - 1; j++) {
      // fix bug here, the windows should contain the node size
      total_cycles += (total_rows + total_cols + node_size);
    }

    total_cycles +=
        total_rows + next_node_size - (elements_steps * total_cols) + node_size;
  }
  total_cycles += num_nodes - (steps * total_rows) + next_node_size -
                  (elements_steps * total_cols) + node_size;
  return total_cycles;
}
void SystolicArray::cycle() {
  if (empty and agg_buffer->isReadReady() and !agg_buffer->isReadBusy() and
      !agg_buffer->isReadEmpty() and output_buffer->isWriteToBufferEmpty()) {
    spdlog::debug("systolic array start a new task,cycle:{}",
                  global_definitions.cycle);
    // generate the output buffer request.
    current_sliding_window = agg_buffer->getReadWindow();
    auto req = std::make_shared<Req>();
    req->the_final_request = current_sliding_window->isTheFinalCol();
    req->the_final_request_of_the_layer =
        current_sliding_window->isTheFinalColOfTheLayer();

    req->addr = current_sliding_window->getOutputAddr();
    req->len = current_sliding_window->getOutputLen();
    req->t = device_types::output_buffer;
    req->req_type = mem_request::write;
    output_buffer->start_write_to_buffer(req);

    // calculate the remaining cycle
    assert(remaining_cycle == 0);
    empty = false;
    running = true;

    remaining_cycle = cal_remaining_cycle();
    global_definitions.do_systolic += remaining_cycle;
    // start agg buffer read
    agg_buffer->start_read();

    spdlog::debug("start a new systolic task: window:{}, total_cycle:{}, "
                  "current cycle:{}",
                  *current_sliding_window, remaining_cycle,
                  global_definitions.cycle);

  } else {
    if (!agg_buffer->isReadReady()) {
      global_definitions.total_waiting_agg_read++;
    }
    if (!output_buffer->isWriteToBufferEmpty()) {
      global_definitions.total_waiting_out++;
    }
  }

  if (remaining_cycle != 0) {
    remaining_cycle--;
    if (remaining_cycle == 0) {
      spdlog::debug("end a systolic array, window:{}, current_cycle:{}",
                    *current_sliding_window, global_definitions.cycle);
      current_sliding_window = nullptr;

      agg_buffer->finish_read();
      output_buffer->finished_write_to_buffer();

      empty = true;
      running = false;
      finished = true;
    }
  }
}
