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
#include <Hash_table.h>
namespace fast_sched {
using pool = output_pool;
using node = output_node;
using work = current_working_window;

enum class edge_info_stats {
  empty,
  reading_edges_and_build_hash,
  finished_build_hash,
  finished_use_hash
};
class controll_info_generator {
public:
  controll_info_generator(const Graph &m_graph,
                          std::vector<unsigned int> nodeDims,
                          std::vector<unsigned int> inputNodesNum)
      : m_pool(m_graph), m_work(inputNodesNum[0], nodeDims[0] * 4),
        m_inputNodeNum(inputNodesNum), final_layer(nodeDims.size()),
        m_hash_table(config::hash_table_size) {}

  unsigned get_current_generation_sequence() const { return sequence; }
  void cycle();

private:
  pool m_pool;
  work m_work;
  std::vector<unsigned int> m_nodeDims;

  std::vector<unsigned> m_inputNodeNum;

  unsigned calculate_insert_node(const node &new_node);
  unsigned per_query_cycle = 2;
  unsigned insert_cycle_per_node = 2;

  bool pool_all_finished = false;

  unsigned sequence = 0;
  unsigned currentLayer = 0;
  unsigned final_layer;
  // this should be set at contstructor
  unsigned next_sequence_remaining_cycle = 0;
  bool working = false;
  sjq::hash_table m_hash_table;
};

struct agg_signal {
  unsigned remaining_cycle = 0;
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
  std::queue<agg_signal> m_agg_signal;

  // the signal generator!
  controll_info_generator m_controll_info_generator;

  // the sequence number we are going to wait, if current we are going to use
  // sequence 0, the number is signal generator should be at least 1!!!
  unsigned current_sequence_number = 1;

public:
  [[nodiscard]] bool isAllFinished() const;
};
} // namespace fast_sched

#endif // GCN_SIM_CONTROLLER_H
