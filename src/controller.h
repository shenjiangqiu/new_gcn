//
// Created by Jiangqiu shen on 5/25/21.
//

#ifndef GCN_SIM_CONTROLLER_H
#define GCN_SIM_CONTROLLER_H
// controller will controll all components
#include "Aggregator_fast.h"
#include "SystolicArray_fast.h"
#include "buffer_fast.h"

#include "fast_sched.h"
namespace fast_sched {
class controller {

  using pool = output_pool;
  using node = output_node;
  using work = current_working_window;

public:
  // constructor
  controller(const Graph &m_graph, const shared_ptr<InputBuffer> &iBf,
             std::vector<unsigned int> nodeSizes,
             std::vector<unsigned int> inputNodesNum,
             std::vector<unsigned int> outputNodeNum,
             std::shared_ptr<Aggregator_fast> agg,
             std::shared_ptr<memory_interface> mMem);
  controller() = delete;

  // operation
  void cycle();

private:
  pool m_current_pool;
  work m_current_work;

  std::shared_ptr<InputBuffer> i_bf;

  std::shared_ptr<Aggregator_fast> agg;
  std::shared_ptr<memory_interface> m_mem;

  // all window is end;
  bool pool_all_finished = false;
  std::vector<unsigned> nodeSizes;
  unsigned totalNodes;
  // the start address of the current input
  uint64_t currentInputBaseAddr;
  // how many features per node in this layer
  unsigned currentNodeSize;
  unsigned currentLayer;
  unsigned finalLayer;
  bool all_finished = false;

  std::vector<unsigned> m_outputNodeNum;
  std::vector<unsigned> m_inputNodeNum;

public:
  [[nodiscard]] bool isAllFinished() const;
};
} // namespace fast_sched

#endif // GCN_SIM_CONTROLLER_H
