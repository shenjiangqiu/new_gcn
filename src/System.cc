//
// Created by sjq on 1/6/21.
//

#include "System.h"

#include <globals.h>
#include <memory>
#include <utility>

#include "debug_helper.h"
#include "spdlog/spdlog.h"
#include "utils/Options.h"

System::System(int inputBufferSize, int edgeBufferSize, int aggBufferSize,
               int outputBufferSize, int aggTotalCores, int systolic_rows,
               int systolic_cols, std::shared_ptr<Graph> graph,
               std::vector<int> node_dim, const std::string &dram_config_name,
               std::shared_ptr<Model> mModel,
               std::shared_ptr<sparseVector> m_vec, bool enable_sparse) {
  this->enable_sparse = enable_sparse;
  global_definitions.m_vec = m_vec;
  global_definitions.enable_sparse = enable_sparse;
  m_vec = m_vec;
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

  GCN_INFO("set up dram_gap:{},cpu_gap:{}, with dram_freq:{},cpu_freq:{}",
           dram_gap, cpu_gap, config::dram_freq, config::core_freq);
  // step1, first need to get the max x_w;
  int total_level = node_dim.size();

  std::vector<int> xw_s;
  std::vector<int> yw_s;
  // for the first layer
  if (m_model->isConcatenate()) {
    // the aggregator's result will concatenate the origin node.
    auto size =
        ((agg_buffer_size / 2) /
         ((2 * node_dim[0] - config::ignore_neighbor - config::ignore_self) *
          4));
    xw_s.push_back(size);
  } else {
    xw_s.push_back((agg_buffer_size / 2) /
                   ((node_dim[0] - config::ignore_neighbor) * 4));
  }

  GCN_INFO("xws push back:{}", xw_s.back());
  assert(xw_s.back() > 0 && "the window should be positive");
  if (config::enable_feature_sparsity) {
    int effective_size = (node_dim[0] - config::ignore_neighbor);
    effective_size = effective_size * (1.0 - config::feature_sparse_rate0);
    yw_s.push_back((input_buffer_size / 2) / (effective_size * 4));
  } else {
    yw_s.push_back((input_buffer_size / 2) /
                   ((node_dim[0] - config::ignore_neighbor) * 4));
  }
  assert(yw_s.back() > 0 && "the window should be positive");
  GCN_INFO("yws push back:{}", yw_s.back());
  average_window_size.resize(total_level);
  for (auto i = 1; i < total_level - 1; i++) {
    if (m_model->isConcatenate()) {
      // the aggregator's result will concatenate the origin node.
      auto size = ((agg_buffer_size / 2) / ((2 * node_dim[i]) * 4));
      xw_s.push_back(size);
      average_window_size[i].update(size);
    } else {
      auto size = (agg_buffer_size / 2) / ((node_dim[i]) * 4);
      xw_s.push_back(size);
      average_window_size[i].update(size);
    }
    GCN_INFO("xws push back:{}", xw_s.back());
    assert(xw_s.back() > 0 && "the window should be positive");

    int effective_node_dim = node_dim[i];
    if (config::enable_feature_sparsity) {
      switch (i) {
      case 0:
        effective_node_dim =
            (int)(effective_node_dim * (1 - config::feature_sparse_rate0));
        break;
      case 1:
        effective_node_dim =
            (int)(effective_node_dim * (1 - config::feature_sparse_rate1));
        break;
      case 2:
        effective_node_dim =
            (int)(effective_node_dim * (1 - config::feature_sparse_rate2));
        break;
      default:;
      }
    }

    yw_s.push_back((input_buffer_size / 2) / (effective_node_dim * 4));
    assert(yw_s.back() > 0 && "the window should be positive");
    GCN_INFO("yws push back:{}", yw_s.back());
  }

  for (auto i = 0; i < total_level - 1; i++) {
    global_definitions.layer_input_windows.push_back(0);
    global_definitions.layer_edges.push_back(0);
    global_definitions.layer_input_vertics.push_back(0);
    global_definitions.layer_do_aggregate.push_back(0);
    global_definitions.layer_wait_input.push_back(0);
    global_definitions.layer_aggregate_op.push_back(0);
    global_definitions.layer_do_systolic.push_back(0);
    global_definitions.layer_window_avg_agg.push_back(0);
    global_definitions.layer_window_avg_input.push_back(0);
  }

  GCN_INFO("build the window: dense:{}", (bool)config::enable_dense_window);
  // step 2, build the windows set. and input buffer,edge buffer
  m_slide_window_set = std::make_shared<dense_window_set>(
      m_graph, xw_s, yw_s, node_dim, total_level, config::enable_dense_window);

  current_iter = m_slide_window_set->begin();
  prev_iter = current_iter;

  input_buffer =
      std::make_shared<InputBuffer>("input_buffer", m_slide_window_set);
  edge_buffer = std::make_shared<EdgeBuffer>("edge buffer", m_slide_window_set);
  m_aggregator = std::make_shared<Aggregator>(input_buffer, edge_buffer,
                                              agg_buffer, agg_total_cores);

  // output the number of the memory read requests need to be read
  uint64_t total_size = 0;
  auto first_layer_size = (node_dim[0] - config::ignore_neighbor) * 4;
  total_size += first_layer_size * m_graph->get_num_nodes() *
                (m_graph->get_num_nodes() + xw_s[0] - 1 / xw_s[0]);
  fmt::print("{} {} {}\n", first_layer_size, m_graph->get_num_nodes(), xw_s[0]);
  for (auto i = 1u; i < node_dim.size() - 1; i++) {
    total_size += node_dim[i] * 4 * m_graph->get_num_nodes() *
                  m_graph->get_num_nodes() / xw_s[i];
  }
  GCN_INFO("total_nodes:{}, total_feature_length:{},total input traffic "
           "need to be read:{}",
           m_graph->get_num_nodes(),
           fmt::join(node_dim.begin(), std::prev(node_dim.end()), ","),
           total_size);
}
class kv_maps {
  std::map<std::string, int> int_map;
  std::map<std::string, float> float_map;
  std::map<std::string, unsigned> unsigned_map;
  std::map<std::string, uint64_t> ull_map;
  std::map<std::string, std::string> str_map;

public:
  void push(std::string k, int v) { int_map.insert({k, v}); }
  void push(std::string k, float v) { float_map.insert({k, v}); }
  void push(std::string k, unsigned v) { unsigned_map.insert({k, v}); }
  void push(std::string k, uint64_t v) { ull_map.insert({k, v}); }
  void push(std::string k, std::string v) { str_map.insert({k, v}); }
  void print() {
    for (auto i : int_map) {
      GCN_INFO("{} : {}", i.first, i.second);
    }
    for (auto i : float_map) {
      GCN_INFO("{} : {}", i.first, i.second);
    }
    for (auto i : unsigned_map) {
      GCN_INFO("{} : {}", i.first, i.second);
    }
    for (auto i : ull_map) {
      GCN_INFO("{} : {}", i.first, i.second);
    }
    for (auto i : str_map) {
      GCN_INFO("{} : {}", i.first, i.second);
    }
  }
};

void System::run() {

  while (true) {
    cycle();
    global_definitions.cycle++;
    if (global_definitions.finished) {
      // finishing up
      print();

      break;
    }
  }
  std::cout << "finished run the simulator, cycle:" << global_definitions.cycle
            << std::endl;
  std::cout << "average_window_size: " << average_window_size[0].get_average()
            << std::endl;
  kv_maps map;

  map.push(std::string("total_Aggregator_idle_waiting_input"),
           global_definitions.total_waiting_input);
  map.push(std::string("total_Aggregator_idle_waiting_edge"),
           global_definitions.total_waiting_edge);
  map.push(std::string("total_idle_waiting_agg_write"),
           global_definitions.total_waiting_agg_write);
  map.push(std::string("do_aggregate"), global_definitions.do_aggregate);
  map.push(std::string("total_aggregate_op"),
           global_definitions.total_aggregate_op);
  map.push(std::string("total_handle_edges"), global_definitions.total_edges);
  map.push(std::string("total_input_windows"),
           global_definitions.total_input_windows);
  map.push(std::string("do_systolic"), global_definitions.do_systolic);
  map.push(std::string("total_systolicArray_idle_waiting_agg_read"),
           global_definitions.total_waiting_agg_read);
  map.push(std::string("total_idle_waiting_out"),
           global_definitions.total_waiting_out);

  map.push(std::string("total_mac_in_systolic_array"),
           global_definitions.total_mac_in_systolic_array);
  map.push(std::string("total_read_input_traffic"),
           global_definitions.total_read_input_traffic);
  map.push(std::string("total_read_edge_traffic"),
           global_definitions.total_read_edge_traffic);
  map.push(std::string("total_inputBuffer_idle_cycles"),
           global_definitions.inputBuffer_idle_cycles);
  map.push(std::string("total_edgeBuffer_idle_cycles"),
           global_definitions.edgeBuffer_idle_cycles);
  map.push(std::string("total_real_input_idle"),
           global_definitions.total_input_buffer_idle);
  map.push(std::string("total_real_edge_idle"),
           global_definitions.total_edge_buffer_idle);
  map.push(std::string("total_cycles"), global_definitions.cycle);

  map.print();

  std::cout << "\nEdgeBuffer total_latency "
            << global_definitions.total_read_edge_latency << " len "
            << global_definitions.total_read_edge_len << " times "
            << global_definitions.total_read_edge_times << " avg_latency "
            << global_definitions.total_read_edge_latency /
                   global_definitions.total_read_edge_times
            << " avg_len "
            << global_definitions.total_read_edge_len /
                   global_definitions.total_read_edge_times
            << "\n";

  std::cout << "InputBuffer total_latency "
            << global_definitions.total_read_input_latency << " len "
            << global_definitions.total_read_input_len << " times "
            << global_definitions.total_read_input_times << " avg_latency "
            << global_definitions.total_read_input_latency /
                   global_definitions.total_read_input_times
            << " avg_vertices "
            << global_definitions.total_read_input_vertices_cnt /
                   global_definitions.total_read_input_times
            << " avg_len "
            << global_definitions.total_read_input_len /
                   global_definitions.total_read_input_times
            << "\n\n";

  GCN_INFO("layer_completion_time  {}",
           fmt::join(global_definitions.finished_time_stamp.begin(),
                     global_definitions.finished_time_stamp.end(), "  "));

  auto layer_time = global_definitions.finished_time_stamp;
  int total_level_r = layer_time.size();

  // fix bug here, finished time_stamp[0] is alread layer time
  // if finished_time_stamp is [1,2]
  // the layer time should be [1-0,2-1]
  // so we need to calculate f[1]-f[0], i=1, end, so i begin at size-1, end at 1
  // before: i = total_level //
  for (int i = total_level_r - 1; i > 0; i--) {
    layer_time[i] = global_definitions.finished_time_stamp[i] -
                    global_definitions.finished_time_stamp[i - 1];
  }

  GCN_INFO("layer_input_windows  {}",
           fmt::join(global_definitions.layer_input_windows.begin(),
                     global_definitions.layer_input_windows.end(), "  "));

  GCN_INFO("layer_edges  {}",
           fmt::join(global_definitions.layer_edges.begin(),
                     global_definitions.layer_edges.end(), "  "));

  GCN_INFO("layer_input_vertics  {}",
           fmt::join(global_definitions.layer_input_vertics.begin(),
                     global_definitions.layer_input_vertics.end(), "  "));

  GCN_INFO("layer_time  {}",
           fmt::join(layer_time.begin(), layer_time.end(), "  "));

  GCN_INFO("layer_wait_input_time  {}",
           fmt::join(global_definitions.layer_wait_input.begin(),
                     global_definitions.layer_wait_input.end(), "  "));

  GCN_INFO("layer_aggregate_time  {}",
           fmt::join(global_definitions.layer_do_aggregate.begin(),
                     global_definitions.layer_do_aggregate.end(), "  "));

  GCN_INFO("layer_systolic_time  {}",
           fmt::join(global_definitions.layer_do_systolic.begin(),
                     global_definitions.layer_do_systolic.end(), "  "));

  GCN_INFO("layer_aggregate_op  {}",
           fmt::join(global_definitions.layer_aggregate_op.begin(),
                     global_definitions.layer_aggregate_op.end(), "  "));

  for (auto i = 0; i < total_level_r; i++) {
    auto cnt = global_definitions.layer_input_windows[i];
    auto agg_time = global_definitions.layer_do_aggregate[i];
    auto input_time = global_definitions.layer_wait_input[i];
    float avg_agg_time = cnt == 0 ? 0 : agg_time / cnt;

    float avg_input_time = cnt == 0 ? 0 : input_time / cnt;
    global_definitions.layer_window_avg_agg[i] = avg_agg_time;
    global_definitions.layer_window_avg_input[i] = avg_input_time;
  }

  GCN_INFO("layer_window_avg_input_lat  {}",
           fmt::join(global_definitions.layer_window_avg_input.begin(),
                     global_definitions.layer_window_avg_input.end(), "  "));

  GCN_INFO("layer_window_avg_agg_lat  {}",
           fmt::join(global_definitions.layer_window_avg_agg.begin(),
                     global_definitions.layer_window_avg_agg.end(), "  "));

  fmt::print("total_read_traffic: {}\n", global_definitions.total_read_traffic);
  fmt::print("total_write_traffic: {}\n",
             global_definitions.total_write_traffic);
  fmt::print("total_read_traffic_input: {}\n",
             global_definitions.total_read_traffic_input);

  fmt::print("total_read_traffic_edge: {}\n",
             global_definitions.total_read_traffic_edge);
}

void System::cycle() {
  // if there are 1000 cycle nothing happen, then a dead lock warning will be
  // triggered it is the 1000 cycle after the last busy cycle
  static uint64_t dead_lock_detect = 10000;
  static unsigned total_detected_times = 0;
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
  if (input_idle and edge_idle) {
    global_definitions.all_buffer_idle++;
    // now test if it's dead lock
    if (global_definitions.cycle >= dead_lock_detect) {
      total_detected_times++;

      dead_lock_detect += 100000;
      // print out dead lock infomation
      GCN_ERROR("Possible dead lock detected! current cycle:{}",
                dead_lock_detect);
      GCN_ERROR("currnet stats:\ninput buffer:{}\n edge_buffer: {}\n "
                "agg_buffer:{} \n out_buffer:{}\n ",
                input_buffer->get_line_trace(), edge_buffer->get_line_trace(),
                agg_buffer->get_line_trace(), output_buffer->get_line_trace());
      if (total_detected_times == 30) {
        std::cout.flush();
        std::cerr.flush();
        throw;
      }
    }
  } else {
    // it's busy!
    dead_lock_detect = global_definitions.cycle + 100000;
  }

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
      GCN_DEBUG("input buffer send req:{},{}", *req, global_definitions.cycle);
      m_mem->send(req);
    } else if (!edge_buffer->isCurrentEmpty() and
               !edge_buffer->isCurrentSent()) {
      auto req = edge_buffer->pop_current();
      GCN_DEBUG("edge buffer send req:{},{}", (*req), global_definitions.cycle);
      m_mem->send(req);
    } else if (!input_buffer->isNextEmpty() and !input_buffer->isNextSent()) {
      auto req = input_buffer->pop_next();
      GCN_DEBUG("input_next buffer send req:{},{}", (*req),
                global_definitions.cycle);
      m_mem->send(req);
    } else if (!edge_buffer->isNextEmpty() and !edge_buffer->isNextSent()) {
      auto req = edge_buffer->pop_next();
      GCN_DEBUG("edge_next buffer send req:{},{}", (*req),
                global_definitions.cycle);
      m_mem->send(req);
    } else if (!output_buffer->isWriteToMemoryEmpty() and
               !output_buffer->isWriteToMemoryStarted()) {
      auto req = output_buffer->popWriteToMemReq();
      GCN_DEBUG("output buffer send req:{},{}", (*req),
                global_definitions.cycle);
      m_mem->send(req);
    }
  }

  if (m_mem->ret_available()) {
    auto ret = m_mem->get_req();
    if (ret->t == device_types::input_buffer) {
      GCN_DEBUG(" input  req received,req:{},cycle:{} ", *ret,
                global_definitions.cycle);

      input_buffer->receive(ret);
    } else if (ret->t == device_types::edge_buffer) {

      GCN_DEBUG(" edge  req received,req:{},cycle:{} ", *ret,
                global_definitions.cycle);

      edge_buffer->receive(ret);
    } else {
      // do not report write done
      assert(ret->t == device_types::output_buffer);
      output_buffer->finished_write_memory();
      GCN_DEBUG("output buffer finished write to memory: cycle:{}",
                global_definitions.cycle);
    }
  } /*
   // add new task
   if (m_aggregator->isEmpty()) {
     // every time we find it empty, just remove current buffer, move next
   buffer
     // to current and run
     GCN_DEBUG("{}:{},the aggregator is emtpy,cycle:{}",
                   global_definitions.cycle);
     // this is a copy
     auto j = *current_iter;
     auto i = *current_iter;
     j++;

     m_aggregator->add_task(std::make_shared<dense_window>(*i));
     // load the prefetch data
     input_buffer->finish_current_move_next();

     // agg_buffer->finish_current_move_next();
     // now need to prefetch
     if (j != m_slide_window_set->end()) {
       GCN_DEBUG(
           "{}:{},the aggregator is emtpy,fetch the next input,cycle:{}",
            global_definitions.cycle);

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
       GCN_DEBUG("{}:{},the aggregator is emtpy,fetch the next output and "
                     "agg,cycle:{}",
                      global_definitions.cycle);

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
     current_iter = std::make_shared<dense_window_iter>(j);
   }*/
}

// print out all element's status
void System::print() const { fmt::print("{}\n", m_mem->get_final_result()); }
