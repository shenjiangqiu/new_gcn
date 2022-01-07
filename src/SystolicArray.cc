//
// Created by sjq on 1/6/21.
//

#include "SystolicArray.h"

#include "Slide_window.h"
#include "debug_helper.h"
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
  uint64_t total_cycles = 0;
  auto num_nodes = current_sliding_window->getXw();
  auto node_dim =
      current_sliding_window->getCurrentnodeDim(); // num elements in one node;
  auto level = current_sliding_window->getLevel();

  // might be confused here
  // 1, get the model
  // 2, the model.level[0] means the level 1, so we always get the next level
  // when using current level.
  auto next_node_dim =
      global_definitions.m_models.at(std::string(config::model))
          .getMLevels()
          .at(current_sliding_window->getLevel());

  if (global_definitions.concate) {
    node_dim *= 2;
  }
  // first layer, we should ignore the
  if (current_sliding_window->getLevel() == 0) {
    // the first layer, we should remove the ignored feature
    node_dim -= config::ignore_neighbor;

    if (global_definitions.concate) {
      node_dim -= config::ignore_self;
    }
    assert(node_dim > 0);
  }
  if (node_dim <= 0) {
    GCN_ERROR("node size < 0 happend,concate:{}, origin node "
              "size:{},ignore_nei:{},ignore_self:{},level:{}",
              global_definitions.concate,
              current_sliding_window->getCurrentnodeDim(),
              config::ignore_neighbor, config::ignore_self);
    spdlog::flush_on(spdlog::level::err);
    throw std::runtime_error("can't check the size");
  }

  // to this now, the node size, next_node_dim and num_nodes are finishied.
  global_definitions.total_mac_in_systolic_array +=
      num_nodes * node_dim * next_node_dim;

  // fix bug here

  auto steps = (total_rows + num_nodes - 1) / total_rows;

  auto elements_steps = (next_node_dim + total_cols - 1) / total_cols;
  for (auto i = 0u; i < steps - 1; i++) {
    for (auto j = 0u; j < elements_steps - 1; j++) {
      // fix bug here, the windows should contain the node size
      total_cycles += (total_rows + total_cols + node_dim);
      total_cycles += (total_rows * total_cols / 4) / 32;
    }

    if (total_rows + next_node_dim - ((elements_steps - 1) * total_cols) +
            node_dim <=
        0) {
      GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
                "total_rows:{},next_node_dim:{},elements_steps*total_cols:"
                "{},node_dim:{}",
                steps, elements_steps, total_rows, next_node_dim,
                elements_steps * total_cols, node_dim);
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("wrong cycle happened!");
    }
    auto remaining_rows = total_rows;
    auto remaining_cols = next_node_dim - ((elements_steps - 1) * total_cols);
    total_cycles += remaining_rows + remaining_cols + node_dim;
    total_cycles += (total_rows * total_cols / 4) / 32;
  }
  for (auto j = 0u; j < elements_steps - 1; j++) {
    // calculate the last row
    auto remaining_rows = num_nodes - ((steps - 1) * total_rows);
    auto remaining_cols = total_cols;
    total_cycles += remaining_rows + remaining_cols + node_dim;
    total_cycles += (total_rows * total_cols / 4) / 32;
    if (remaining_rows <= 0 or remaining_rows > total_rows) {
      GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
                "total_rows:{},next_node_dim:{},elements_steps*total_cols:"
                "{},node_dim:{}",
                steps, elements_steps, total_rows, next_node_dim,
                elements_steps * total_cols, node_dim);
    }
  }
  // calculate the last row the last col
  auto remaining_rows = num_nodes - ((steps - 1) * total_rows);
  auto remaining_cols = next_node_dim - ((elements_steps - 1) * total_cols);

  total_cycles += remaining_rows + remaining_cols + node_dim;
  total_cycles += (total_rows * total_cols / 4) / 32;

  if (remaining_rows <= 0 or remaining_cols <= 0 or
      remaining_rows > total_rows or remaining_cols > total_cols) {
    GCN_ERROR("wrong cycle happened: steps:{},elements_steps:{}\n "
              "total_rows:{},next_node_dim:{},elements_steps*total_cols:"
              "{},node_dim:{}",
              steps, elements_steps, total_rows, next_node_dim,
              elements_steps * total_cols, node_dim);
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
    // fix bug here, because each col will only apeare once, so only the first
    // row will be here.so do not need to test if it's the last row.
    // old:current_sliding_window->isTheFinalCol() and
    // current_sliding_window->isTheFinalRow()

    req->the_final_request_of_the_layer =
        current_sliding_window->isTheFinalCol();
    if (config::enable_dense_window)
      GCN_DEBUG("window:{}, final_request:{}, final_request_of_row:{}",
                *std::static_pointer_cast<dense_window>(current_sliding_window),
                req->the_final_request, req->the_final_request_of_the_layer);
    else
      GCN_DEBUG("window:{}, final_request:{}, final_request_of_row:{}",
                *std::static_pointer_cast<Slide_window>(current_sliding_window),
                req->the_final_request, req->the_final_request_of_the_layer);

    if (config::enable_sparse) {

      // approximate here, ignore the last one to avoid overflow
      // because our get_start_addr_masks() do not support the element after the
      // last one
      auto x_start = current_sliding_window->getX();
      auto x_end = x_start + current_sliding_window->getXw() - 1;

      // culculate the write address range:
      auto layer = current_sliding_window->getLevel();
      auto addr_start =
          global_definitions.m_vec->get_start_addr_masks(layer, x_start);
      auto addr_end =
          global_definitions.m_vec->get_start_addr_masks(layer, x_end);
      auto len = addr_end - addr_start;
      req->set_addr(addr_start, len);
    } else {
      req->set_addr(current_sliding_window->getOutputAddr(),
                    current_sliding_window->getOutputLen());
    }
    req->t = device_types::output_buffer;
    req->req_type = mem_request::write;
    output_buffer->start_write_to_buffer(req);

    // calculate the remaining cycle
    assert(remaining_cycle == 0);
    empty = false;
    running = true;
    if (config::enable_sparse) {
      remaining_cycle =
          global_definitions.m_vec->getMultiplyCycles(256) / ((256 * 30));

      fmt::print("systolic array cycle:{}\n", remaining_cycle);
      global_definitions.do_systolic += remaining_cycle;

    } else {
      remaining_cycle = cal_remaining_cycle();
      fmt::print("systolic array cycle:{}\n", remaining_cycle);

      global_definitions.do_systolic += remaining_cycle;
      // start agg buffer read
    }
    agg_buffer->start_read();

  } else {
    // fix bug here!
    // only when it's empy we can consider it's idle and count
    if (empty) {
      // it's empty but cannot serve! find the reason!!
      if (!agg_buffer->isReadReady()) {
        global_definitions.total_waiting_agg_read++;
      }
      if (!output_buffer->isWriteToBufferEmpty()) {
        global_definitions.total_waiting_out++;
      }
    }
  }

  if (remaining_cycle != 0) {
    remaining_cycle--;
    if (remaining_cycle == 0) {

      current_sliding_window = nullptr;

      agg_buffer->finish_read();
      output_buffer->finished_write_to_buffer();

      empty = true;
      running = false;
      finished = true;
    }
  }
}
