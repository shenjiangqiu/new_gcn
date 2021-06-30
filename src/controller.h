//
// Created by Jiangqiu shen on 5/25/21.
//

#ifndef GCN_SIM_CONTROLLER_H
#define GCN_SIM_CONTROLLER_H
// controller will controll all components
#include "Aggregator_fast.h"
#include "SystolicArray_fast.h"
#include "buffer_fast.h"
#include "contoll_signal_generator.hh"
#include "fast_sched.h"
#include <Hash_table.h>
namespace fast_sched {
using pool = output_pool;
using node = output_node;
using work = current_working_window;
/// this class will maintain a queue, to store currently know shortest nodes
/// operations:
/// @author Jiangqiu Shen
/// @details this class will maintain three operations
///
/// @line 1. insert a new node into the buffer, args, node id, and size
/// @line 2. finihsed a new node, args, node id, vector of edges
/// @line 3. query the current shortest
/// current policy: 1, the queue is always sorted, 2 queue is not sorted,search
/// all entry to select the smallest, 3. the queue is not sorted, random select
class shortest_node_info_generator {
  struct queue_entry {
    unsigned node;
    unsigned size;
  };

public:
  explicit shortest_node_info_generator(unsigned policy, unsigned queue_size);
  // will return the cycle
  unsigned get_next_shortest(unsigned &shortest_value);
  unsigned finished_output(unsigned node_id, const std::vector<unsigned> &edges,
                           bool is_final);

private:
  std::vector<queue_entry> current_choose_vector;
  unsigned current_policy;
  unsigned queue_size;
};

class controller {

public:
  // constructor
  controller(const Graph &m_graph, const shared_ptr<InputBuffer> &iBf,
             std::vector<unsigned int> nodeDims,
             std::vector<unsigned int> inputNodesNum,
             std::shared_ptr<Aggregator_fast> agg,
             std::shared_ptr<memory_interface> mMem);
  controller() = delete;

  // operation
  void cycle();

private:
  pool m_current_pool;
  work m_current_work;

  std::shared_ptr<InputBuffer> i_bf;

  // all window is end;
  bool pool_all_finished = false;
  std::vector<unsigned> nodeDims;
  unsigned totalNodes;
  // the start address of the current input
  uint64_t currentInputBaseAddr;
  // how many features per node in this layer
  unsigned currentnodeDim;
  unsigned currentLayer;
  unsigned finalLayer;
  bool all_finished = false;

  std::vector<unsigned> m_inputNodeNum;
  std::shared_ptr<Aggregator_fast> agg;
  std::shared_ptr<memory_interface> m_mem;

  // generated signal buffered here

  // the signal generator!
  control_info_generator m_controll_info_generator;

  // the sequence number we are going to wait, if current we are going to use
  // sequence 0, the number is signal generator should be at least 1!!!
  unsigned current_sequence_number = 1;

public:
  [[nodiscard]] bool isAllFinished() const;
};
} // namespace fast_sched

#endif // GCN_SIM_CONTROLLER_H
