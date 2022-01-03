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
#include "edge_hash.h"
#include "fast_sched.h"
#include <Hash_table.h>
#include <deque>
#include <sparseVector.h>
#include <utils/average_number.hh>
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
/// @line 3. query_and_delete the current shortest
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
struct agg_task {
  std::vector<unsigned> input_nodes;
  unsigned total_edges;
  std::vector<std::pair<unsigned, std::vector<unsigned>>> edges;
  std::vector<std::pair<unsigned, std::vector<unsigned>>> reverse_edges;
  unsigned current_layer;
};

class controller {

public:
  // constructor
  controller(const Graph &m_graph, const shared_ptr<InputBuffer> &iBf,
             std::vector<unsigned int> nodeDims,
             std::vector<unsigned int> inputNodesNum,
             std::shared_ptr<Aggregator_fast> agg,
             std::shared_ptr<memory_interface> mMem,
             unsigned int shortLargeDivider, unsigned int shortQueueSize,
             unsigned int largeQueueSize, unsigned int taskQueueSize,
             unsigned int aggBufferSize, bool, std::string,
             std::vector<std::string> in_names,
             std::vector<std::string> mask_names);
  controller() = delete;
  // operation
  void cycle();
  [[nodiscard]] bool isAllFinished() const;
  void print();

private:
  void handle_insert_queue(bool is_short);
  sparseVector m_vec;
  // store a tmepory edge when insert fail
  bool next_to_insert_valid{false};
  std::pair<unsigned, unsigned> next_to_insert_edge{0, 0};

  void handle_remaining_cycle();
  void handle_buffer_relative_cycle();
  void handle_task_generation();
  void handle_work_insert();
  std::queue<agg_task> task_generation_queue{};

  // 1,2 is short , >3 is large
  unsigned short_large_divider;

  // this two queue should be maintainced during insert to hash
  std::deque<unsigned> short_queue{};
  std::deque<unsigned> large_queue{};

  std::set<std::pair<unsigned, unsigned>> all_tasks_pool;

  sjq::edge_hash hashtable1;
  sjq::hash_table hashTable2;
  bool need_to_insert = true;
  unsigned short_queue_size;
  unsigned large_queue_size;

  unsigned task_queue_size;

  unsigned remaining_cycle_build_task{};
  unsigned remaining_cycle_insert_hash{};

  pool m_current_pool;

  std::shared_ptr<InputBuffer> i_bf;

  // all window is end;
  bool pool_all_finished = false;
  std::vector<unsigned> nodeDims;
  unsigned totalNodes;
  // the start address of the current input
  uint64_t currentInputBaseAddr;
  // how many features per node in this layer
  unsigned currentnodeDim;

  // this is the layer the agg and ibfer runing layer
  unsigned current_running_layer = 0;

  // this is the layer the pool generate edges. this value might be more
  // advanced than current_running_layer
  unsigned currentLayer = 0;
  unsigned finalLayer;
  bool all_finished = false;

  std::vector<unsigned> m_inputNodeNum;
  std::vector<unsigned> m_inputNodeDims;

  std::shared_ptr<Aggregator_fast> agg;
  std::shared_ptr<memory_interface> m_mem;

  unsigned aggBufferSize;

  // generated signal buffered here

  // the signal generator!
  // control_info_generator m_controll_info_generator;

  // the sequence number we are going to wait, if current we are going to use
  // sequence 0, the number is signal generator should be at least 1!!!
  unsigned current_sequence_number = 1;

  unsigned unable_to_insert_hashtable_full1 = 0;
  unsigned unable_to_insert_hashtable_full2 = 0;
  unsigned unable_to_insert_aggbuffer_full = 0;
  std::vector<average_number> average_window_size;
};

} // namespace fast_sched

#endif // GCN_SIM_CONTROLLER_H
