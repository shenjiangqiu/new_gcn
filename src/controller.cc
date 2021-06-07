//
// Created by Jiangqiu shen on 5/25/21.
//

#include "controller.h"
#include "debug_helper.h"
#include "spdlog/spdlog.h"
#include <utility>
namespace fast_sched {

// note here, we do not count the latency of read csc format edge from dram to
// on-chip buffer, because the bottleneck should be signal generation process
void fast_sched::controll_info_generator::cycle() {

  // 1. when have work, do work

  if (working)
    if (next_sequence_remaining_cycle > 0) {
      next_sequence_remaining_cycle--;
      if (next_sequence_remaining_cycle == 0) {
        // this task finished
        sequence++;
        working = false;
      }
      return;
    }
  // not working

  // should add new task because remaining cycle is 0

  // 2, not working, from working_window to generate working set
  if (m_work.have_next_input_node()) {
    // get the next input req and send it to the Inputbuffer
    auto valid_nodes = m_work.get_num_valid_work();
    auto next_input_nodes = m_work.get_next_input_nodes();
    auto total_query = valid_nodes * next_input_nodes.size();

    // noticed here, why we use += not =, because might need additional cycle:
    // insert a new output node!!!! read the next stage.
    next_sequence_remaining_cycle += total_query * per_query_cycle;
    working = true;
  }

  // 3, when there are empty slotes in working window, add them in
  if (!m_work.getAllFinishedCol().empty() and m_pool.have_next_col()) {
    for (auto &&i : m_work.getAllFinishedCol()) {
      if (m_pool.have_next_col()) {
        auto next_input_line = m_pool.get_next_input_line();
        auto line_size = next_input_line.getInputNodes().size();
        m_work.add(i, next_input_line);
        next_sequence_remaining_cycle += line_size * insert_cycle_per_node;
      } else {
        break;
      }
    }
  }

  // switch to next layer
  if (!m_pool.have_next_col() and not pool_all_finished) {
    // waiting until all current layer finished
    if (!m_work.have_next_input_node()) {
      // move to next layer
      // should insert a new layer
      GCN_INFO("in_controll_info_generator,switch to next_layer:{}",
               currentLayer + 1);

      currentLayer++;
      if (currentLayer == final_layer) {
        GCN_INFO("finished the pool:current layer:{}", currentLayer);
        pool_all_finished = true;
      } else {
        GCN_INFO("switch to next layer:layer:{}", currentLayer);
        m_pool.reset();
        // set up the environments
        assert(currentLayer < nodeDims.size());

        m_work =
            work(m_outputNodeNum[currentLayer], m_inputNodeNum[currentLayer]);
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
  // 4, check working window
  // 5, check pool
  // 6, send memory request to memory
  // 7, check if everything is finished
  // 8, change ibuffer stats

  // 1, send read request to input buffer
  if (i_bf->getNextState() == InputBufferState::empty and
      m_current_work.have_next_input_node()) {
    // get the next input req and send it to the Inputbuffer
    auto req = std::make_shared<Req>();
    auto next_input_nodes = m_current_work.get_next_input_nodes();
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

  // 4, check working window
  if (!m_current_work.getAllFinishedCol().empty() and
      m_current_pool.have_next_col()) {
    for (auto &&i : m_current_work.getAllFinishedCol()) {
      if (m_current_pool.have_next_col()) {
        m_current_work.add(i, m_current_pool.get_next_input_line());
      } else {
        break;
      }
    }
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
        GCN_INFO("setup new window: outputNodes:{},inputNodes:{}",
                 m_outputNodeNum[currentLayer], m_inputNodeNum[currentLayer]);
        m_current_work =
            work(m_outputNodeNum[currentLayer], m_inputNodeNum[currentLayer]);
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
                       std::vector<unsigned int> outputNodeNum,
                       std::shared_ptr<Aggregator_fast> agg,
                       std::shared_ptr<memory_interface> mMem)
    : m_current_pool(m_graph),
      m_current_work(outputNodeNum[0], inputNodesNum[0]), i_bf(iBf),
      nodeDims(std::move(nodeDims)), totalNodes(m_graph.get_num_nodes()),
      currentInputBaseAddr(0x00ff00), currentnodeDim(this->nodeDims[0]),
      currentLayer(0), finalLayer(this->nodeDims.size()),
      m_outputNodeNum(outputNodeNum), m_inputNodeNum(inputNodesNum),
      agg(std::move(agg)), m_mem(std::move(mMem)),
      m_controll_info_generator(m_graph, this->nodeDims, inputNodesNum,
                                outputNodeNum) {}
bool controller::isAllFinished() const { return all_finished; }
} // namespace fast_sched
