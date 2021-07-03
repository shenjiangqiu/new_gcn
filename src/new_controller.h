//
// Created by Jiangqiu shen on 6/30/21.
//

#ifndef GCN_SIM_NEW_CONTROLLER_H
#define GCN_SIM_NEW_CONTROLLER_H
#include "Hash_table.h"
#include "utils/sized_queue.h"
#include "graph.h"
#include "vector"
#include "Aggregator_fast.h"
#include "buffer_fast.h"


namespace sjq {
using namespace fast_sched;


class Task{
  unsigned num_total_edges;
  std::vector<unsigned> input_nodes;

};

class new_controller {

public:
  new_controller();

  void cycle();


private:
  std::shared_ptr<Graph> m_graph;
  hash_table m_hashtable_1;
  hash_table m_hashtable_2;

  std::queue<unsigned> long_queue;
  std::queue<unsigned> short_queue;

  unsigned agg_buffer_size;

  std::vector<unsigned> layer_input_node_num_to_read;
  std::shared_ptr<Aggregator_fast> agg;
  std::vector<unsigned> nodeDims;
  std::shared_ptr<InputBuffer> i_bf;

  std::queue<







};
}

#endif // GCN_SIM_NEW_CONTROLLER_H
