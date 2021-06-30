#ifndef CONTOLL_SIGNAL_GENERATOR_HH
#define CONTOLL_SIGNAL_GENERATOR_HH
#include <Hash_table.h>
#include <fast_sched.h>
#include <graph.h>
#include <memory>
#include <utils/common.hh>
#include <utils/sized_queue.h>
#include <vector>
// the real signal send from info generator to agg controller
struct signal {};

class control_info_generator {
public:
  control_info_generator(
      const Graph &m_graph, std::vector<unsigned int> nodeDims,
      std::vector<unsigned int> inputNodesNum,
      std::shared_ptr<sized_queue<signal>> task_sending_queue)
      : m_pool(m_graph), m_work(inputNodesNum[0], nodeDims[0] * 4),
        m_nodeDims(nodeDims), m_inputNodeNum(inputNodesNum),
        final_layer(nodeDims.size()), m_hash_table_1(config::hash_table_size),
        m_hash_table_2(config::hash_table_size),
        m_task_sending_queue(task_sending_queue) {}

  [[nodiscard]] unsigned get_current_generation_sequence() const {
    return sequence;
  }
  void cycle();

private:
  fast_sched::output_pool m_pool;
  fast_sched::current_working_window m_work;
  std::vector<unsigned int> m_nodeDims;

  std::vector<unsigned> m_inputNodeNum;

  bool pool_all_finished = false;

  unsigned sequence = 0;
  unsigned currentLayer = 0;
  unsigned final_layer;
  // this should be set at contstructor
  unsigned next_sequence_remaining_cycle = 0;
  bool working = false;
  sjq::hash_table m_hash_table_1; // used for normal edge
  sjq::hash_table m_hash_table_2; // used for reverse edge

  std::queue<unsigned> small_nodes_queue;
  std::queue<unsigned> large_nodes_queue;

  std::shared_ptr<sized_queue<signal>> m_task_sending_queue;
};

#endif /* CONTOLL_SIGNAL_GENERATOR_HH */
