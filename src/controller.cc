//
// Created by Jiangqiu shen on 5/25/21.
//

#include "controller.h"
#include "debug_helper.h"
#include "spdlog/spdlog.h"
#include <utility>

#define WINDOW_PRINT_GAP 100

#ifndef NDEBUG

#include <map>

std::map<unsigned, std::vector<unsigned>> window_compare;
#endif
namespace fast_sched {
void print_window(const current_working_window &m_current_work,
                  const std::vector<unsigned> &next_input_nodes,
                  uint64_t print_sign, const char *name) {
  GCN_INFO("the {} th input window in CONTOLLER_{} is : {} "
           ",current_output_size:{},input_size:{}, "
           "agg_usage:{},edge_usage:{},current_output_node_size:{}",
           print_sign, name, fmt::join(next_input_nodes, ","),
           m_current_work.get_output_size(), m_current_work.get_input_size(),
           m_current_work.get_agg_usage(), m_current_work.get_edge_usage(),
           m_current_work.get_current_output_node_size());
#ifndef NDEBUG
  if (window_compare.contains(print_sign)) {
    assert(window_compare.at(print_sign) == next_input_nodes);

    GCN_INFO("find {} , match!!", print_sign);
    window_compare.erase(print_sign);
  } else {
    window_compare.insert({print_sign, next_input_nodes});
    GCN_INFO("insert: {} with {}", print_sign,
             fmt::join(next_input_nodes, ","));
  }

#endif
}
// note here, we do not count the latency of read csc format edge from dram to
// on-chip buffer, because the bottleneck should be signal generation process
void fast_sched::control_info_generator::cycle() {

  // 1. when have work, do work

  if (working)
    if (next_sequence_remaining_cycle > 0) {
      next_sequence_remaining_cycle--;
      if (next_sequence_remaining_cycle == 0) {
        // this task finished
        sequence++;
        if (sequence % 100 == 0) {
          GCN_INFO("finished_controller_info_window:{}", sequence);
        }
        working = false;
      }
      return;
    }
  // not working

  // should add new task because remaining cycle is 0

  // 2, not working, from working_window to generate working set
  if (m_work.have_next_input_node()) {
    // get the next input req and send it to the Inputbuffer
    // auto valid_nodes = m_work.get_num_valid_work();
    auto next_input_nodes = m_work.get_next_input_nodes();
    // auto total_query = valid_nodes * next_input_nodes.size();
    static uint64_t print_sign = 0;
    if (print_sign % WINDOW_PRINT_GAP == 0) {
      print_window(m_work, next_input_nodes, print_sign, "info");
    }
    print_sign++;
    // noticed here, why we use += not =, because might need additional cycle:
    // insert a new output node!!!! read the next stage.

    // update here, real query
    auto cycle = 1u;
    for (auto input : next_input_nodes) {

      cycle += m_hash_table.query(input);
      global_definitions.total_cycle_query_hash_table += cycle;
    }
    global_definitions.total_cycle_query_hash_table += cycle;

    next_sequence_remaining_cycle += cycle;

    working = true;
  }

  // 3, when there are empty slotes in working window, add them in
  while (m_pool.have_next_col() and
         m_work.can_add(m_pool.get_next_input_line())) {

    auto next_input_line = m_pool.get_next_input_line_and_move();
    // auto line_size = next_input_line.getInputNodes().size();
    m_work.add(next_input_line);

    // update here, real insert
    auto cycle = 0u;
    for (auto input : next_input_line.get_not_processed()) {

      auto ret = m_hash_table.insert(input);
      global_definitions.total_cycle_insert_hash_table += ret;
      if (ret == 0) {
        spdlog::error("fail to insert a line? this line should be handled "
                      "by manuelly serch, might try to insert it again?");
        throw std::runtime_error("error");
      }
      cycle += ret;
      global_definitions.total_cycle_insert_hash_table += ret;
    }

    next_sequence_remaining_cycle += cycle;
  }

  // switch to next layer
  if (!m_pool.have_next_col() and not pool_all_finished) {
    // waiting until all current layer finished
    if (!m_work.have_next_input_node()) {
      // move to next layer
      // should insert a new layer
      GCN_INFO("in_controller_signal:in_controll_info_generator,switch to "
               "next_layer:{}",
               currentLayer + 1);

      currentLayer++;

      if (currentLayer == final_layer) {
        GCN_INFO("in_controller_signal:finished the pool:current layer:{}",
                 currentLayer);
        pool_all_finished = true;
      } else {
        GCN_INFO("in_controller_signal:switch to next layer:layer:{}",
                 currentLayer);
        m_pool.reset();
        // set up the environments
        // fix bug here, the nodesize should be 4xdim
        m_work =
            work(m_inputNodeNum[currentLayer], 4 * m_nodeDims[currentLayer]);
      }
    }
  }
}

void controller::cycle() {
  // update the controller signal

  // need to sync with the controller signal
  m_controll_info_generator.cycle();
  // 1, send read request to input buffer
  // 2, send task to agg
  // 3, check mem
  // 4, check working window, fix bug here, remenber to add lines to
  // working_window 5, check pool 6, send memory request to memory 7, check if
  // everything is finished 8, change ibuffer stats

  // 1, send read request to input buffer
  if (i_bf->getNextState() == InputBufferState::empty and
      m_current_work.have_next_input_node()) {
    // get the next input req and send it to the Inputbuffer
    auto req = std::make_shared<Req>();
    auto next_input_nodes = m_current_work.get_next_input_nodes();
    static uint64_t print_sign = 0;
    if (print_sign % WINDOW_PRINT_GAP == 0) {
      print_window(m_current_work, next_input_nodes, print_sign, "self");
    }
    print_sign++;

    // from nodes to address
    std::vector<uint64_t> addrs;
    // from nodes to addres
    for (auto &&i : next_input_nodes) {
      uint64_t start_addr = i * currentnodeDim * 4 + currentInputBaseAddr;
      uint64_t end_addr = start_addr + currentnodeDim * 4;
      while (start_addr < end_addr) {
        addrs.push_back(start_addr);
        start_addr += 64;
      }
    }
    req->set_addr(addrs);
    req->t = device_types::input_buffer;
    req->req_type = mem_request::read;
    req->items_cnt = m_current_work.get_current_item_count();
    req->nodeDim = currentnodeDim;
    i_bf->send(req);
  }

  // 2, send task to agg
  // fix bug here,
  // fix again , this not a bug, the edges to be read is obviousely, no need to
  // waiting the control signal, just ignore it!!!
  if (i_bf->getCurrentState() == InputBufferState::readyToRead and
      !agg->isWorking()) {
    // start point, current_sequence_number=1,
    // m_controll_info_generator.get_current_generation_sequence()=0, will need
    // to wait the first control signal.
    if (current_sequence_number >
        m_controll_info_generator.get_current_generation_sequence()) {
      // controller signal not ready, wait!! note that the controller signal
      // generation is fully async with agg Do nothing
    } else {
      current_sequence_number++;
      if (current_sequence_number % 100 == 0) {
        GCN_INFO("finished_controller sequence:{}", current_sequence_number);
      }
      auto &req = i_bf->getCurrentReq();
      assert(req->nodeDim > 0);
      agg->add_task(req, req->nodeDim);
      i_bf->setCurrentState(InputBufferState::reading);
    }
  }
  // 3, check mem

  if (m_mem->ret_available()) {
    assert(m_mem->peek_req()->t == device_types::input_buffer);
    i_bf->receive(m_mem->get_req());
  }
  // insert line to work
  while (m_current_pool.have_next_col() and
         m_current_work.can_add(m_current_pool.get_next_input_line())) {
    auto &&nd = m_current_pool.get_next_input_line_and_move();

    static uint64_t print_sign = 0;
    if (print_sign % 10 == 0) {
      global_definitions.edge_agg_logger->info("edge_buffer: {} {}",
                                               m_current_work.get_edge_usage(),
                                               (int)config::edgeSize);
      global_definitions.edge_agg_logger->info("agg_buffer: {} {}",
                                               m_current_work.get_agg_usage(),
                                               (int)config::aggSize);
    }

    print_sign++;

    m_current_work.add(nd);
  }

  // 5, check pool
  if (!m_current_pool.have_next_col() and not pool_all_finished) {
    // waiting until all current layer finished
    if (!m_current_work.have_next_input_node()) {
      // move to next layer
      currentLayer++;
      if (currentLayer == finalLayer) {
        GCN_INFO("finished the pool:current layer:{}", currentLayer);
        pool_all_finished = true;
      } else {
        GCN_INFO("switch to next layer:layer:{}", currentLayer);
        m_current_pool.reset();
        // set up the environments
        currentInputBaseAddr += currentnodeDim * 4 * totalNodes;
        assert(currentLayer < nodeDims.size());
        currentnodeDim = nodeDims[currentLayer];
        GCN_INFO("setup new window: inputNodes:{} node_size:{}",
                 m_inputNodeNum[currentLayer], nodeDims[currentLayer] * 4);
        m_current_work =
            work(m_inputNodeNum[currentLayer], 4 * nodeDims[currentLayer]);
      }
    }
  }

  // 6, send mem
  if (m_mem->available()) {
    if (i_bf->getCurrentState() == InputBufferState::readyToSend) {
      m_mem->send(i_bf->getCurrentReq());
      i_bf->setCurrentState(InputBufferState::sent);
    } else if (i_bf->getNextState() == InputBufferState::readyToSend) {
      m_mem->send(i_bf->getNextReq());
      i_bf->setNextState(InputBufferState::sent);
    }
  }

  // 7, check finished
  if (pool_all_finished and
      i_bf->getCurrentState() == InputBufferState::empty and
      i_bf->getNextState() == InputBufferState::empty and !agg->isWorking()) {
    all_finished = true;
  }

  // 8, change ibuffer stats
  if (!agg->isWorking() and
      i_bf->getCurrentState() == InputBufferState::reading) {
    i_bf->setCurrentState(InputBufferState::empty);
  }
}

controller::controller(const Graph &m_graph, const shared_ptr<InputBuffer> &iBf,
                       std::vector<unsigned int> nodeDims,
                       std::vector<unsigned int> inputNodesNum,
                       std::shared_ptr<Aggregator_fast> agg,
                       std::shared_ptr<memory_interface> mMem)
    : m_current_pool(m_graph),
      m_current_work(inputNodesNum[0], 4 * nodeDims[0]), i_bf(iBf),
      nodeDims(std::move(nodeDims)), totalNodes(m_graph.get_num_nodes()),
      currentInputBaseAddr(0x00ff00), currentnodeDim(this->nodeDims[0]),
      currentLayer(0), finalLayer(this->nodeDims.size()),
      m_inputNodeNum(inputNodesNum), agg(std::move(agg)),
      m_mem(std::move(mMem)),
      m_controll_info_generator(m_graph, this->nodeDims, inputNodesNum) {}
bool controller::isAllFinished() const { return all_finished; }

shortest_node_info_generator::shortest_node_info_generator(unsigned int policy,
                                                           unsigned queue_size)
    : current_policy(policy), queue_size(queue_size) {}

unsigned
shortest_node_info_generator::get_next_shortest(unsigned int &shortest_value) {
  unsigned cycle = 0;
  if (current_policy == 0) {
    // will sort
    auto &entry = current_choose_vector.front();
    shortest_value = entry.node;
    cycle += 1;
  } else if (current_policy == 1) {
    // not sorted
    unsigned shortest = 0;
    for (auto i = 0; i < current_choose_vector.size(); i++) {
      if (current_choose_vector[i].size <
          current_choose_vector[shortest].size) {

        shortest = i;
      }
    }
    shortest_value = current_choose_vector[shortest].node;
    cycle += queue_size;
  }
  return cycle;
}
unsigned shortest_node_info_generator::finished_output(
    unsigned int node_id, const std::vector<unsigned int> &edges,
    bool is_final) {
  if(current_policy==0){
    // the queue is ordered, only re-sort when some node will be removed
    if(is_final){
      // this is the final one, need to resort all


    }else{
      // not the final one?? what to do?
    }
  }

  return 0;
}


}