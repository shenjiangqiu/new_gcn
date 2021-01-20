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
               int systolic_cols, std::shared_ptr<Graph> graph,
               std::vector<int> node_size, const std::string &dram_config_name,
               std::shared_ptr<Model> mModel)
    : input_buffer_size(inputBufferSize), edge_buffer_size(edgeBufferSize),
      agg_buffer_size(aggBufferSize), output_buffer_size(outputBufferSize),
      agg_total_cores(aggTotalCores),
      agg_buffer(std::make_shared<Aggregator_buffer>("agg_buffer")),

      output_buffer(std::make_shared<WriteBuffer>("output_buffer")),

      m_systolic_array(std::make_shared<SystolicArray>(
          systolic_rows, systolic_cols, agg_buffer, output_buffer)),
      m_graph(std::move(graph)),
      m_mem(std::make_shared<memory_interface>(dram_config_name, 64)),
      m_model(mModel) {
  // first need to get the max x_w;
  int total_level = node_size.size();

  std::vector<int> xw_s;
  std::vector<int> yw_s;
  for (auto i = 0; i < total_level - 1; i++) {
    if (m_model->isConcatenate()) {
      // the aggregator's result will concatenate the origin node.
      auto size = ((agg_buffer_size / 4) / (node_size[i] * 4));
      xw_s.push_back(size);
    } else {
      xw_s.push_back((agg_buffer_size / 2) / (node_size[i] * 4));
    }
    spdlog::info("xws push back:{}", xw_s.back());
    assert(xw_s.back() > 0 && "the window should be positive");
    yw_s.push_back((input_buffer_size / 2) / (node_size[i] * 4));
    assert(yw_s.back() > 0 && "the window should be positive");
    spdlog::info("yws push back:{}", yw_s.back());
  }
  m_slide_window_set = std::make_shared<Slide_window_set>(
      m_graph, xw_s, yw_s, node_size, total_level);
  current_iter =
      std::make_shared<slide_window_set_iterator>(m_slide_window_set->begin());
  prev_iter = std::make_shared<slide_window_set_iterator>(*current_iter);

  input_buffer =
      std::make_shared<InputBuffer>("input_buffer", m_slide_window_set);
  edge_buffer = std::make_shared<EdgeBuffer>("edge buffer", m_slide_window_set);
  m_aggregator = std::make_shared<Aggregator>(input_buffer, edge_buffer,
                                              agg_buffer, agg_total_cores);
}
void System::run() {

  while (!finished) {
    cycle();
    global_definitions.cycle++;
    if (global_definitions.finished) {
      break;
    }
  }
  std::cout << "finished run the simulator, cycle:" << global_definitions.cycle
            << std::endl;
  spdlog::info("the result\n"
               "total_waiting_input {}\n"
               "total_waiting_edge {}\n"
               "total_waiting_agg_write {}\n"
               "do_aggregate {}\n"
               "do_systolic {}\n"
               "total_waiting_agg_read {}\n"
               "total_waiting_out {}\n"
               "total_read_input_latency {}\n"
               "total_read_input_times {}\n"
               "total_read_edge_latency {}\n"
               "total_read_edge_times {}\n",
               global_definitions.total_waiting_input,
               global_definitions.total_waiting_edge,
               global_definitions.total_waiting_agg_write,
               global_definitions.do_aggregate, global_definitions.do_systolic,
               global_definitions.total_waiting_agg_read,
               global_definitions.total_waiting_out,
               global_definitions.total_read_input_latency,
               global_definitions.total_read_input_times,
               global_definitions.total_read_edge_latency,
               global_definitions.total_read_edge_times);
}
void System::cycle() {
  input_buffer->cycle();
  edge_buffer->cycle();
  agg_buffer->cycle();
  output_buffer->cycle();

  m_aggregator->cycle();
  m_systolic_array->cycle();
  // TODO, the memory working frequency is not same as gcn_sim
  m_mem->cycle();

  // handle memory requests
  // connection between buffer and
  if (m_mem->avaliable()) {
    if (!input_buffer->isCurrentEmpty() and !input_buffer->isCurrentSent()) {
      auto req = input_buffer->pop_current();
      spdlog::debug("{}:{},input buffer send req:{},{}", __FILE__, __LINE__,
                    *req, global_definitions.cycle);
      m_mem->send(req);
    } else if (!edge_buffer->isCurrentEmpty() and
               !edge_buffer->isCurrentSent()) {
      auto req = edge_buffer->pop_current();
      spdlog::debug("{}:{},edge buffer send req:{},{}", __FILE__, __LINE__,
                    (*req), global_definitions.cycle);
      m_mem->send(req);
    } else if (!input_buffer->isNextEmpty() and !input_buffer->isNextSent()) {
      auto req = input_buffer->pop_next();
      spdlog::debug("{}:{},input_next buffer send req:{},{}", __FILE__,
                    __LINE__, (*req), global_definitions.cycle);
      m_mem->send(req);
    } else if (!edge_buffer->isNextEmpty() and !edge_buffer->isNextSent()) {
      auto req = edge_buffer->pop_next();
      spdlog::debug("{}:{},edge_next buffer send req:{},{}", __FILE__, __LINE__,
                    (*req), global_definitions.cycle);
      m_mem->send(req);
    } else if (!output_buffer->isWriteToMemoryEmpty() and
               !output_buffer->isWriteToMemoryStarted()) {
      auto req = output_buffer->popWriteToMemReq();
      spdlog::debug("{}:{},output buffer send req:{},{}", __FILE__, __LINE__,
                    (*req), global_definitions.cycle);
      m_mem->send(req);
    }
  }

  if (m_mem->ret_avaliable()) {
    auto ret = m_mem->get_req();
    if (ret->t == device_types::input_buffer) {
      spdlog::debug(" input  req received,req:{},cycle:{} ", *ret,
                    global_definitions.cycle);

      input_buffer->receive(ret);
    } else if (ret->t == device_types::edge_buffer) {

      spdlog::debug(" edge  req received,req:{},cycle:{} ", *ret,
                    global_definitions.cycle);

      edge_buffer->receive(ret);
    } else {
      // do not report write done
      assert(ret->t == device_types::output_buffer);
      output_buffer->finished_write_memory();
      spdlog::debug("output buffer finished write to memory: cycle:{}",
                    global_definitions.cycle);
    }
  } /*
   // add new task
   if (m_aggregator->isEmpty()) {
     // every time we find it empty, just remove current buffer, move next
   buffer
     // to current and run
     spdlog::debug("{}:{},the aggregator is emtpy,cycle:{}", __FILE__, __LINE__,
                   global_definitions.cycle);
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
       spdlog::debug(
           "{}:{},the aggregator is emtpy,fetch the next input,cycle:{}",
           __FILE__, __LINE__, global_definitions.cycle);

       // prefetch the next input buffer
       auto next_input_req = std::make_shared<Req>();
       next_input_req->addr = (*j).getInputAddr();
       next_input_req->len = (*j).getInputLen();
       next_input_req->req_type = mem_request::read;
       next_input_req->t = device_types::input_buffer;
       input_buffer->add_next(next_input_req);
     }

     if (i == m_slide_window_set->begin() or
         (((**prev_iter).getX() != (*i).getX()) or
          (**prev_iter).getLevel() != (*i).getLevel())) {
       // which means, currently we are going to execute a new col.
       // finished current col.
       // prefetch the next col
       spdlog::debug("{}:{},the aggregator is emtpy,fetch the next output and "
                     "agg,cycle:{}",
                     __FILE__, __LINE__, global_definitions.cycle);

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
     prev_iter = current_iter;
     current_iter = std::make_shared<slide_window_set_iterator>(j);
   }*/
}
