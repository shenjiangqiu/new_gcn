//
// Created by Jiangqiu shen on 5/25/21.
//

#include "controller.h"
#include "spdlog/spdlog.h"
#include <utility>
namespace fast_sched {

void controller::cycle() {
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
      uint64_t start_addr = i * currentNodeSize * 4 + currentInputBaseAddr;
      uint64_t end_addr = start_addr + currentNodeSize * 4;
      while (start_addr < end_addr) {
        addrs.push_back(start_addr);
        start_addr += 64;
      }
    }
    req->set_addr(addrs);
    req->t = device_types::input_buffer;
    req->req_type = mem_request::read;
    req->items_cnt = m_current_work.get_current_item_count();
    req->nodeSize = currentNodeSize;
    i_bf->send(req);
  }

  // 2, send task to agg
  if (i_bf->getCurrentState() == InputBufferState::readyToRead and
      !agg->isWorking()) {
    auto& req=i_bf->getCurrentReq();
    assert(req->nodeSize>0);
    agg->add_task(req, req->nodeSize);
    i_bf->setCurrentState(InputBufferState::reading);
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
        pool_all_finished = true;
      } else {
        m_current_pool.reset();
        // set up the environments
        currentInputBaseAddr += currentNodeSize * 4 * totalNodes;
        assert(currentLayer<nodeSizes.size());
        currentNodeSize = nodeSizes[currentLayer];
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
                       std::vector<unsigned int> nodeSizes,
                       std::vector<unsigned int> inputNodesNum,
                       std::vector<unsigned int> outputNodeNum,
                       std::shared_ptr<Aggregator_fast> agg,
                       std::shared_ptr<memory_interface> mMem)
    : m_current_pool(m_graph),
      m_current_work(outputNodeNum[0], inputNodesNum[0]), i_bf(iBf),
      nodeSizes(std::move(nodeSizes)), totalNodes(m_graph.get_num_nodes()),
      currentInputBaseAddr(0x00ff00), currentNodeSize(this->nodeSizes[0]),
      currentLayer(0), finalLayer(this->nodeSizes.size()),
      m_outputNodeNum(outputNodeNum), m_inputNodeNum(inputNodesNum),
      agg(std::move(agg)), m_mem(std::move(mMem)) {}
bool controller::isAllFinished() const { return all_finished; }
} // namespace fast_sched
