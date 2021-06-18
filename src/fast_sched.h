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
#include "fmt/format.h"
#include "graph.h"
#include "map"
#include "set"
#include "sliding_window_interface.h"
#include "string"
#include "vector"

namespace fast_sched {

// represent one vertical line in matrix
class output_node {
  // all the in_edges
public:
  explicit output_node(const std::vector<unsigned> &input_nodes, unsigned id,
                       unsigned edge_size);
  explicit output_node(std::set<unsigned> input_nodes, unsigned id,
                       unsigned edge_size);
  // get the edge buffer requirement of this outout node
  unsigned get_edge_size() const { return edgesize; }
  // get the agg buffer requirement of this output node

  std::string get_line_trace() const {
    std::string out;
    for (auto i : input_nodes) {
      if (not_processed_nodes.count(i)) {
        out += " .";
        out += std::to_string(i);
      } else {
        out += " x";
        out += std::to_string(i);
      }
    }
    return out;
  }
  output_node() = default;
  [[nodiscard]] const std::set<unsigned int> &getInputNodes() const;
  void setInputNodes(const std::set<unsigned int> &inputNodes);

  // this will consume the next n input
  [[nodiscard]] std::vector<unsigned> get_next_n_input(unsigned n) const;
  unsigned invalid_input(const std::vector<unsigned> &input);
  [[nodiscard]] bool is_all_processed() const {
    return not_processed_nodes.empty();
  }
  unsigned get_remaining() const { return not_processed_nodes.size(); }
  bool exists(unsigned id) const { return not_processed_nodes.count(id) > 0; }
  const std::set<unsigned> &get_not_processed() const {
    return not_processed_nodes;
  }
  unsigned get_output_node_id() const { return output_node_id; }

private:
  std::set<unsigned> input_nodes;

  unsigned output_node_id = 0;
  unsigned edgesize = 0;
  // all nodes that is not processed yet
  std::set<unsigned> not_processed_nodes;
  // true: already processed, false: not precessed
};

// contains multiple output_nodes

class current_working_window {
public:
  bool have_next_input_node() { return !current_window.empty(); }
  explicit current_working_window(unsigned numInputCapacity,
                                  unsigned output_node_size);
  // void invalid(unsigned);

  bool can_add(const output_node &node) const;

  void add(const output_node &nd);

  unsigned get_num_valid_work() const { return current_window.size(); }
  // void invalid_and_add(unsigned id, const output_node &nd);
  std::vector<unsigned> get_next_input_nodes();
  std::string get_line_trace() {
    std::string out;
    for (const auto &i : current_window) {
      out += i.second.get_line_trace();
      out += "\n";
    }
    return out;
  }
  unsigned get_current_item_count() { return current_item_count; }

private:
  std::map<unsigned, output_node> current_window;

  // this size if fixed because it's only decided by the input buffer
  unsigned num_input_capacity;
  // the number of agg buffer being used
  unsigned total_agg_buffer_usage;
  // the number of edge buffuer being used
  unsigned total_edge_buffer_usage;
  // number of edges of next input window;
  unsigned current_item_count;

  unsigned current_output_node_size;
};

class output_pool {
public:
  explicit output_pool(const Graph &m_graph);
  // get a new input line
  output_node get_next_input_line() const;
  output_node get_next_input_line_and_move();
  void only_move() { current_position++; }
  void reset() { current_position = 0; }

  bool have_next_col() {
    return current_position < all_remaining_output_nodes.size();
  }
  std::string get_line_trace() {
    std::string out;
    auto count = 0u;
    for (auto i : all_remaining_output_nodes) {
      if (count < current_position) {
        out += "x ";
      } else {
        out += "o ";
      }
      count++;
      out += i.get_line_trace();
      out += "\n";
    }
    return out;
  }

private:
  std::vector<output_node> all_remaining_output_nodes;
  unsigned current_position = 0;
};

} // namespace fast_sched
#endif // GCN_SIM_FAST_SCHED_H
