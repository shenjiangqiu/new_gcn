//
// Created by sjq on 1/6/21.
//

#include "System.h"

#include <globals.h>
#include <memory>
#include <utility>

#include "spdlog/spdlog.h"
#include "utils/Options.h"
System::System(int inputBufferSize, int edgeBufferSize, int aggBufferSize,
               int outputBufferSize, int aggTotalCores, int systolic_rows,
               int systolic_cols, std::string graphName,
               std::vector<int> node_size)
    : input_buffer_size(inputBufferSize), edge_buffer_size(edgeBufferSize),
      agg_buffer_size(aggBufferSize), output_buffer_size(outputBufferSize),
      agg_total_cores(aggTotalCores),
      agg_buffer(std::make_shared<Aggregator_buffer>("agg_buffer")),
      input_buffer(std::make_shared<ReadBuffer>("input_buffer")),
      edge_buffer(std::make_shared<ReadBuffer>("edge buffer")),
      output_buffer(std::make_shared<WriteBuffer>("output_buffer")),
      m_aggregator(std::make_shared<Aggregator>(input_buffer, edge_buffer,
                                                agg_buffer, agg_total_cores)),
      m_systolic_array(std::make_shared<SystolicArray>(
          systolic_rows, systolic_cols, agg_buffer, output_buffer)),
      graph_name(std::move(graphName)),
      m_graph(std::make_shared<Graph>(graph_name)) {
  // first need to get the max x_w;
  int total_level = node_size.size();

  std::vector<int> xw_s;
  std::vector<int> yw_s;
  for (auto i = 0; i < total_level - 1; i++) {
    xw_s.push_back((agg_buffer_size / 2) / node_size[i]);
    yw_s.push_back((input_buffer_size / 2) / node_size[i]);
  }
  m_slide_window_set = std::make_shared<Slide_window_set>(
      m_graph, xw_s, yw_s, node_size, total_level);
  current_iter =
      std::make_shared<slide_window_set_iterator>(m_slide_window_set->begin());
  prev_iter = std::make_shared<slide_window_set_iterator>(*current_iter);
}
void System::run() {
  // init the buffer

  auto begin = m_slide_window_set->get_windows().begin();

  auto input_req = std::make_shared<Req>();

  input_req->addr = begin->getInputAddr();
  input_req->len = begin->getInputLen();
  input_req->req_type = mem_request::read;
  input_req->t = device_types::input_buffer;
  input_buffer->add_next(input_req);

  auto edge_req = std::make_shared<Req>();
  edge_req->addr = begin->getEdgeAddr();
  edge_req->len = begin->getEdgeLen();
  edge_req->req_type = mem_request::read;
  edge_req->t = device_types::edge_buffer;
  edge_buffer->add_next(edge_req);

  while (!finished) {
    cycle();
    global_definitions.cycle++;
  }
  std::cout << "finished run the simulator, cycle:" << global_definitions.cycle
            << std::endl;
}
void System::cycle() {
  input_buffer->cycle();
  edge_buffer->cycle();
  agg_buffer->cycle();
  output_buffer->cycle();

  m_aggregator->cycle();
  m_systolic_array->cycle();
  m_mem->cycle();

  // handle memory requests
  // connection between buffer and
  if (m_mem->avaliable()) {
    if (input_buffer->isCurrentSendReady()) {
      auto req = input_buffer->pop_current_req();
      m_mem->send(req);
    } else if (edge_buffer->isCurrentSendReady()) {
      auto req = edge_buffer->pop_current_req();
      m_mem->send(req);
    } else if (input_buffer->isNextSendReady()) {
      auto req = input_buffer->pop_next_req();
      m_mem->send(req);
    } else if (edge_buffer->isNextSendReady()) {
      auto req = edge_buffer->pop_next_req();
      m_mem->send(req);
    } else if (output_buffer->isNextSendReady()) {
      m_mem->send(output_buffer->pop_next_req());
    } else if (output_buffer->isCurrentSendReady()) {
      m_mem->send(output_buffer->pop_current_req());
    }
  }

  if (m_mem->ret_avaliable()) {
    auto ret = m_mem->get_req();
    if (ret->t == device_types::input_buffer) {
      spdlog::debug(" input  req received,cycle:{} ", global_definitions.cycle);

      input_buffer->accept_req(ret);
    } else {
      assert(ret->t == device_types::edge_buffer);
      spdlog::debug(" edge  req received,cycle:{} ", global_definitions.cycle);

      edge_buffer->accept_req(ret);
    }
  }
  // add new task
  if (m_aggregator->isEmpty()) {
    // every time we find it empty, just remove current buffer, move next buffer
    // to current and run

    // this is a copy
    auto j = *current_iter;
    auto i = *current_iter;
    j++;

    m_aggregator->add_task(std::make_shared<Slide_window>(*i));
    // load the prefetch data
    input_buffer->finish_current_move_next();

    // agg_buffer->finish_current_move_next();
    // now need to prefetch
    if (j != m_slide_window_set->end()) {
      // prefetch the next input buffer
      auto next_input_req = std::make_shared<Req>();
      next_input_req->addr = (*j).getInputAddr();
      next_input_req->len = (*j).getInputLen();
      next_input_req->req_type = mem_request::read;
      next_input_req->t = device_types::input_buffer;
      input_buffer->add_next(next_input_req);
    }
    // TODO: 1,fix the problem of prev and next,2,task if the output buffer
    // succeed send the request,3, need to end the system when to last request
    // is send
    // TODO: remember: the systolic array will automatically start when the
    // agg-buffer's read port is ready!
    // TODO: remember: need the increase the current_iter and prev_iter after
    // this function!!!!
    if (i == m_slide_window_set->begin() or
        (((**prev_iter).getX() != (*i).getX()) or
         (**prev_iter).getLevel() != (*i).getLevel())) {
      // which meas, currently we are going to execute a new col.
      // finished current col.
      // prefetch the next col
      edge_buffer->finish_current_move_next();

      auto next_edge_req = std::make_shared<Req>();
      next_edge_req->addr = (*i.get_next_col()).getEdgeAddr();
      next_edge_req->len = (*i.get_next_col()).getEdgeLen();
      next_edge_req->req_type = mem_request::read;
      next_edge_req->t = device_types::edge_buffer;
      edge_buffer->add_next(next_edge_req);
      if (i != m_slide_window_set->begin())
        agg_buffer->finish_write();
    }
  }
}
