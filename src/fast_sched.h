//
// Created by Jiangqiu shen on 5/21/21.
// fast scheduler
// need a output poll, which can provide the remaining output window
// each window should include the masks which can get unfinished input nodes
//
//

#ifndef GCN_SIM_FAST_SCHED_H
#define GCN_SIM_FAST_SCHED_H
#include "bitset"
#include "graph.h"
#include "map"
#include "set"
#include "string"
#include "vector"
namespace fast_ched {

// represent one vertical line in matrix
class output_node {
  // all the in_edges
public:
  explicit output_node(const std::vector<unsigned> &input_nodes);
  explicit output_node(std::set<unsigned> input_nodes);

  output_node() = default;
  [[nodiscard]] const std::set<unsigned int> &getInputNodes() const;
  void setInputNodes(const std::set<unsigned int> &inputNodes);

  // this will consume the next n input
  [[nodiscard]] std::vector<unsigned> get_next_n_input(unsigned n) const;
  void invalid_input(const std::vector<unsigned> &input);
  [[nodiscard]] bool is_all_processed() const {
    return not_processed_nodes.empty();
  }
  unsigned get_remaining() { return not_processed_nodes.size(); }

private:
  std::set<unsigned> input_nodes;
  // all nodes that is not processed yet
  std::set<unsigned> not_processed_nodes;
  // true: already processed, false: not precessed
};

// contains multiple output_nodes

class current_working_window {
public:
  explicit current_working_window(unsigned int size,
                                  unsigned int numInputCapacity);
  void invalid(unsigned);
  void add(unsigned, const output_node &nd);

  void invalid_and_add(unsigned id, const output_node &nd);
  std::vector<unsigned> get_next_input_nodes();

private:
  unsigned sz;
  std::vector<output_node> current_window;
  std::vector<bool> current_valid;
  unsigned num_input_capacity;
  // those col is empty, need to be replaced!
  std::set<unsigned> all_finished_col;

public:
  [[nodiscard]] const std::set<unsigned int> &getAllFinishedCol() const;
};

class output_poll {
public:
  explicit output_poll(const Graph &m_graph);
  // get a new input line
  const output_node &get_next_input_line();

private:
  std::vector<output_node> all_remaining_output_nodes;
  unsigned current_position = 0;
};

} // namespace fast_ched

#endif // GCN_SIM_FAST_SCHED_H
