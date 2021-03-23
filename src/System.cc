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
               std::shared_ptr<Model> mModel) {

  m_graph = std::move(graph);
  input_buffer_size = inputBufferSize;
  edge_buffer_size = edgeBufferSize;
  agg_buffer_size = aggBufferSize;
  output_buffer_size = outputBufferSize;
  agg_total_cores = aggTotalCores;
  output_buffer = std::make_shared<WriteBuffer>("output_buffer");
  agg_buffer = std::make_shared<Aggregator_buffer>("agg_buffer");

  m_mem = std::make_shared<memory_interface>(dram_config_name,
                                             "HBMDevice4GbLegacy.ini", 64);
  m_systolic_array = std::make_shared<SystolicArray>(
      systolic_rows, systolic_cols, agg_buffer, output_buffer);

  m_model = std::move(mModel);
  cpu_gap = (double)1.0 / (config::core_freq * 1000000000);
  dram_gap = (double)1.0 / (config::dram_freq * 1000000000);

  spdlog::info("set up dram_gap:{},cpu_gap:{}, with dram_freq:{},cpu_freq:{}",
               dram_gap, cpu_gap, config::dram_freq, config::core_freq);
  // step1, first need to get the max x_w;
  int total_level = node_size.size();

  std::vector<int> xw_s;
  std::vector<int> yw_s;
  // for the first layer
  if (m_model->isConcatenate()) {
    // the aggregator's result will concatenate the origin node.
    auto size =
        ((agg_buffer_size / 2) /
         ((2 * node_size[0] - config::ignore_neighbor - config::ignore_self) *
          4));
    xw_s.push_back(size);
  } else {
    xw_s.push_back((agg_buffer_size / 2) /
                   ((node_size[0] - config::ignore_neighbor) * 4));
  }

  spdlog::info("xws push back:{}", xw_s.back());
  assert(xw_s.back() > 0 && "the window should be positive");
  if( config::enable_feature_sparsity ){
     int effective_size = (node_size[0] - config::ignore_neighbor);
     effective_size = effective_size *( 1.0 -config::feature_sparse_rate0);
     yw_s.push_back((input_buffer_size / 2) /
                 ( effective_size * 4));
  }else{
    yw_s.push_back((input_buffer_size / 2) /
                 ((node_size[0] - config::ignore_neighbor) * 4));
  }
  assert(yw_s.back() > 0 && "the window should be positive");
  spdlog::info("yws push back:{}", yw_s.back());

  for (auto i = 1; i < total_level - 1; i++) {
    if (m_model->isConcatenate()) {
      // the aggregator's result will concatenate the origin node.
      auto size = ((agg_buffer_size / 2) / ((2 * node_size[i]) * 4));
      xw_s.push_back(size);
    } else {
      xw_s.push_back((agg_buffer_size / 2) / ((node_size[i]) * 4));
    }
    spdlog::info("xws push back:{}", xw_s.back());
    assert(xw_s.back() > 0 && "the window should be positive");
    
    int effective_node_size = node_size[i];
    if( config::enable_feature_sparsity ){
       switch( i ){
          case 0:
                  effective_node_size = (int)(effective_node_size*
                      (1-config::feature_sparse_rate0));
                  break;    
          case 1:
                  effective_node_size = (int)(effective_node_size*
                      (1-config::feature_sparse_rate1));
                  break;  
          case 2:
                  effective_node_size = (int)(effective_node_size*
                      (1-config::feature_sparse_rate2));
                  break; 
          default:
                   ;
       }
    }

    yw_s.push_back((input_buffer_size / 2) / (effective_node_size * 4));
    assert(yw_s.back() > 0 && "the window should be positive");
    spdlog::info("yws push back:{}", yw_s.back());  
  }

  for( auto i = 0; i < total_level; i++){
    global_definitions.layer_input_windows.push_back(0);
    global_definitions.layer_edges.push_back(0);
    global_definitions.layer_input_vertics.push_back(0);
    global_definitions.layer_do_aggregate.push_back(0);
    global_definitions.layer_aggregate_op.push_back(0);
    global_definitions.layer_do_systolic.push_back(0);
  }

  // step 2, build the windows set. and input buffer,edge buffer
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

  // output the number of the memory read requests need to be read
  uint64_t total_size = 0;
  auto first_layer_size = (node_size[0] - config::ignore_neighbor) * 4;
  total_size += first_layer_size * m_graph->get_num_nodes() *
                (m_graph->get_num_nodes() + xw_s[0] - 1 / xw_s[0]);
  fmt::print("{} {} {}\n", first_layer_size, m_graph->get_num_nodes(), xw_s[0]);
  for (auto i = 1; i < node_size.size() - 1; i++) {
    total_size += node_size[i] * 4 * m_graph->get_num_nodes() *
                  m_graph->get_num_nodes() / xw_s[i];
  }
  spdlog::info("total_nodes:{}, total_feature_length:{},total input traffic "
               "need to be read:{}",
               m_graph->get_num_nodes(),
               fmt::join(node_size.begin(), std::prev(node_size.end()), ","),
               total_size);
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
               "total_Aggregator_idle_waiting_input {}\n"
               "total_Aggregator_idle_waiting_edge {}\n"
               "total_idle_waiting_agg_write {}\n"
               "do_aggregate {}\n"
               "total_aggregate_op {}\n"
               "total_handle_edges {}\n"
               "total_input_windows {}\n"
               "do_systolic {}\n"
               "total_systolicArray_idle_waiting_agg_read {}\n"
               "total_idle_waiting_out {}\n"
               "ReaderBuffer_total_read_input_latency {} "
               "total_read_input_len {}  "
               "total_read_input_times {}  "
               "avg_read_latency {} "
               "avg_read_len {}\n"
               "ReaderBuffer_total_read_edge_latency {}  "
               "ReaderBuffer_total_read_edge_len {}  "
               "total_read_edge_times {}  "
               "avg_read_edge_latency {}  "
               "avg_read_edge_len {}\n"
               "total_mac_in_systolic_array {}\n"
               "total_read_input_traffic {}\n"
               "total_read_edge_traffic {}\n"
               "total_inputBuffer_idle_cycles {}\n"
               "total_edgeBuffer_idle_cycles {}\n"
               "total_real_input_idle {}\n"
               "total_real_edge_idle {}\n"
               "total_cycles {}\n",
               global_definitions.total_waiting_input,
               global_definitions.total_waiting_edge,
               global_definitions.total_waiting_agg_write,
               global_definitions.do_aggregate, 
               global_definitions.total_aggregate_op,
               global_definitions.total_edges,
               global_definitions.total_input_windows,
               global_definitions.do_systolic,
               global_definitions.total_waiting_agg_read,
               global_definitions.total_waiting_out,
               global_definitions.total_read_input_latency,
               global_definitions.total_read_input_len,
               global_definitions.total_read_input_times,
               global_definitions.total_read_input_latency /
                   global_definitions.total_read_input_times,
               global_definitions.total_read_input_len /
                   global_definitions.total_read_input_times,
               global_definitions.total_read_edge_latency,
               global_definitions.total_read_edge_len,
               global_definitions.total_read_edge_times,
               global_definitions.total_read_edge_latency /
                   global_definitions.total_read_edge_times,
               global_definitions.total_read_edge_len /
                   global_definitions.total_read_edge_times,
               global_definitions.total_mac_in_systolic_array,
               global_definitions.total_read_input_traffic,
               global_definitions.total_read_edge_traffic,
               global_definitions.inputBuffer_idle_cycles,
               global_definitions.edgeBuffer_idle_cycles,
               global_definitions.total_input_buffer_idle,
               global_definitions.total_edge_buffer_idle,
               global_definitions.cycle);

  spdlog::info("layer_completion_time  {}\n",
               fmt::join(global_definitions.finished_time_stamp.begin(),
                         global_definitions.finished_time_stamp.end(), "  "));

  spdlog::info("layer_input_windows  {}\n",
               fmt::join(global_definitions.layer_input_windows.begin(),
                         global_definitions.layer_input_windows.end(), "  "));

  spdlog::info("layer_edges  {}\n",
               fmt::join(global_definitions.layer_edges.begin(),
                         global_definitions.layer_edges.end(), "  "));
  
  spdlog::info("layer_input_vertics  {}\n",
               fmt::join(global_definitions.layer_input_vertics.begin(),
                         global_definitions.layer_input_vertics.end(), "  "));

  spdlog::info("layer_do_aggregate(cycles)  {}\n",
               fmt::join(global_definitions.layer_do_aggregate.begin(),
                         global_definitions.layer_do_aggregate.end(), "  "));

  spdlog::info("layer_do_systolic(cycles)  {}\n",
               fmt::join(global_definitions.layer_do_systolic.begin(),
                         global_definitions.layer_do_systolic.end(), "  "));  
  
  spdlog::info("layer_aggregate_op  {}\n",
               fmt::join(global_definitions.layer_aggregate_op.begin(),
                         global_definitions.layer_aggregate_op.end(), "  "));                                              
  
                                              


}

void System::cycle() {

  input_buffer->cycle();
  edge_buffer->cycle();
  agg_buffer->cycle();
  output_buffer->cycle();

  m_aggregator->cycle();
  m_systolic_array->cycle();

  auto input_idle = input_buffer->idle();
  auto edge_idle = edge_buffer->idle();

  if (input_idle)
    global_definitions.total_input_buffer_idle++;
  if (edge_idle)
    global_definitions.total_edge_buffer_idle++;
  if (input_idle and edge_idle)
    global_definitions.all_buffer_idle++;

  // adjust the frequency
  current_system_time += cpu_gap;
  while (current_dram_time < current_system_time) {
    m_mem->cycle();
    current_dram_time += dram_gap;
  }

  // handle memory requests
  // connection between buffer and
  if (m_mem->available()) {
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

  if (m_mem->ret_available()) {
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
