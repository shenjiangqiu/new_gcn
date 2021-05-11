//
// Created by sjq on 1/6/21.
//

#include "SystolicArray.h"

#include "globals.h"
#include "spdlog/spdlog.h"
#include <memory>
#include"debug_helper.h"

SystolicArray::SystolicArray(int totalRows, int totalCols,
                             const shared_ptr<Aggregator_buffer> &aggBuffer,
                             const shared_ptr<WriteBuffer> &outputBuffer)
    : total_rows(totalRows), total_cols(totalCols),
      current_sliding_window(nullptr), agg_buffer(aggBuffer),
      output_buffer(outputBuffer) {}
int SystolicArray::cal_remaining_cycle() {
  // TODO: Add output latency, the weight is decided by output layer dimension.
  assert(current_sliding_window);
  uint64_t total_cycles = 0;
  auto num_nodes = current_sliding_window->getXw();
  auto node_size =
      current_sliding_window->getCurrentNodeSize(); // num elements in one node;
  auto level = current_sliding_window->getLevel();

  // might be confused here
  // 1, get the model
  // 2, the model.level[0] means the level 1, so we always get the next level
  // when using current level.
  auto next_node_size =
      global_definitions.m_models.at(std::string(config::model))
          .getMLevels()
          .at(current_sliding_window->getLevel());

  if (global_definitions.concate) {
    node_size *= 2;
  }
  // first layer, we should ignore the
  if (current_sliding_window->getLevel() == 0) {
    // the first layer, we should remove the ignored feature
    node_size -= config::ignore_neighbor;

    if (global_definitions.concate) {
      node_size -= config::ignore_self;
    }
    assert(node_size > 0);
  }
  if (node_size <= 0) {
    GCN_ERROR("node size < 0 happend,concate:{}, origin node "
                  "size:{},ignore_nei:{},ignore_self:{},level:{}",
                  global_definitions.concate,
                  current_sliding_window->getCurrentNodeSize(),
                  config::ignore_neighbor, config::ignore_self);
    spdlog::flush_on(spdlog::level::err);
    throw std::runtime_error("can't check the size");
  }

  // to this now, the node size, next_node_size and num_nodes are finishied.
  global_definitions.total_mac_in_systolic_array +=
      num_nodes * node_size * next_node_size;

  // fix bug here

  auto steps = (total_rows + num_nodes - 1) / total_rows;

  auto elements_steps = (next_node_size + total_cols - 1) / total_cols;
  for (auto i = 0u; i < steps - 1; i++) {
    for (auto j = 0u; j < elements_steps - 1; j++) {
      // fix bug here, the windows should contain the node size
      total_cycles += (total_rows + total_cols + node_size);
      total_cycles += (total_rows * total_cols / 4) / 32;
    }

    if (total_rows + next_node_size - ((elements_steps - 1) * total_cols) +
            node_size <=
        0) {
      GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
                    "total_rows:{},next_node_size:{},elements_steps*total_cols:"
                    "{},node_size:{}",
                    steps, elements_steps, total_rows, next_node_size,
                    elements_steps * total_cols, node_size);
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("wrong cycle happened!");
    }
    auto remaining_rows = total_rows;
    auto remaining_cols = next_node_size - ((elements_steps - 1) * total_cols);
    total_cycles += remaining_rows + remaining_cols + node_size;
    total_cycles += (total_rows * total_cols / 4) / 32;
  }
  for (auto j = 0u; j < elements_steps - 1; j++) {
    // calculate the last row
    auto remaining_rows = num_nodes - ((steps - 1) * total_rows);
    auto remaining_cols = total_cols;
    total_cycles += remaining_rows + remaining_cols + node_size;
    total_cycles += (total_rows * total_cols / 4) / 32;
    if (remaining_rows <= 0 or remaining_rows > total_rows) {
      GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
                    "total_rows:{},next_node_size:{},elements_steps*total_cols:"
                    "{},node_size:{}",
                    steps, elements_steps, total_rows, next_node_size,
                    elements_steps * total_cols, node_size);
    }
  }
  // calculate the last row the last col
  auto remaining_rows = num_nodes - ((steps - 1) * total_rows);
  auto remaining_cols = next_node_size - ((elements_steps - 1) * total_cols);

  total_cycles += remaining_rows + remaining_cols + node_size;
  total_cycles += (total_rows * total_cols / 4) / 32;

  if (remaining_rows <= 0 or remaining_cols <= 0 or
      remaining_rows > total_rows or remaining_cols > total_cols) {
    GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
                  "total_rows:{},next_node_size:{},elements_steps*total_cols:"
                  "{},node_size:{}",
                  steps, elements_steps, total_rows, next_node_size,
                  elements_steps * total_cols, node_size);
    spdlog::flush_on(spdlog::level::err);
    throw std::runtime_error("wrong cycle happened!");
  }

  global_definitions.layer_do_systolic[level] += total_cycles;

  return total_cycles;
}
void SystolicArray::cycle() {
  if (empty and agg_buffer->isReadReady() and !agg_buffer->isReadBusy() and
      !agg_buffer->isReadEmpty() and output_buffer->isWriteToBufferEmpty()) {
    GCN_DEBUG("systolic array start a new task,cycle:{}",
                  global_definitions.cycle);
    // generate the output buffer request.
    current_sliding_window = agg_buffer->getReadWindow();
    auto req = std::make_shared<Req>();
    req->the_final_request = current_sliding_window->isTheFinalCol() and
                             current_sliding_window->isTheFinalLayer();

    req->the_final_request_of_the_layer =
        current_sliding_window->isTheFinalRow() and
        current_sliding_window->isTheFinalCol();

    req->set_addr(current_sliding_window->getOutputAddr(),
                  current_sliding_window->getOutputLen());
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

    GCN_DEBUG("start a new systolic task: window:{}, total_cycle:{}, "
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
      GCN_DEBUG("end a systolic array, window:{}, current_cycle:{}",
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
