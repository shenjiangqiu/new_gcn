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
#include <boost/range/adaptor/indexed.hpp>

namespace fast_sched {

// represent one vertical line in matrix
// Update: shoud support partial node add to current_window
class output_node {
  // all the in_edges
public:
  void reset() { current_position = 0; }
  // TODO, 1.should be able to add or remove edges
  // TODO, 2. should support real mark(mark the size, when take, should to test
  // if the node is really valid!!!!)
  unsigned get_total_len() const { return not_processed_nodes.size(); }
  void add_edge(unsigned input_id);
  unsigned pop() {
    auto ret = not_processed_nodes.at(current_position);
    current_position++;
    return ret;
  }
  unsigned top() const { return not_processed_nodes.at(current_position); }

  explicit output_node(const std::vector<unsigned> &input_nodes, unsigned id);
  explicit output_node(std::set<unsigned> input_nodes, unsigned id);
  // get the edge buffer requirement of this outout node
  // get the agg buffer requirement of this output node

  output_node() = default;
  // [[maybe_unused]] [[nodiscard]] const std::set<unsigned int> &
  // getInputNodes() const;

  // [[maybe_unused]] void setInputNodes(const std::set<unsigned int>
  // &inputNodes);

  // this will consume the next n input
  // No longer needed
  // [[nodiscard]] std::vector<unsigned> get_next_n_input(unsigned n) const;

  // No longer needed
  // unsigned invalid_input(const std::vector<unsigned> &input);

  [[nodiscard]] bool is_all_processed() const {
    return current_position == not_processed_nodes.size();
  }
  [[nodiscard]] unsigned get_remaining() const {
    return not_processed_nodes.size() - current_position;
  }

  [[nodiscard]] const std::vector<unsigned> &get_not_processed() const {
    return not_processed_nodes;
  }
  [[nodiscard]] unsigned get_output_node_id() const { return output_node_id; }

private:
  unsigned current_position = 0;
  unsigned output_node_id;

  // all nodes that is not processed yet
  const std::vector<unsigned> not_processed_nodes;
  // the next position to accessed when call top and pop, when call pop, this
  // value should be added
  unsigned remaining_valid_edges = 0;
  // this is used for test  and validation only!!
};

// contains multiple output_nodes

// TODO: need to change the logic, now the node may not be full, the partial
// node should be maintained to be insert later
class current_working_window {
public:
  bool have_next_input_node() { return !current_window.empty(); }
  explicit current_working_window(unsigned numInputCapacity,
                                  unsigned output_node_size);
  // void invalid(unsigned);
  // TODO now need to change this, can_add should be removed because the
  // hashtable will be the only reason it cannot add DELETE [[nodiscard]] bool
  // can_add(const output_node &node) const;

  // fix here, now just add a new edge
  void add(unsigned output, unsigned input);

  [[maybe_unused]] [[nodiscard]] unsigned get_num_valid_work() const {
    return current_window.size();
  }
  // void invalid_and_add(unsigned id, const output_node &nd);
  std::vector<unsigned> get_next_input_nodes();

  [[nodiscard]] unsigned get_current_item_count() const {
    return current_item_count;
  }
  [[nodiscard]] unsigned get_output_size() const {
    return current_window.size();
  }
  [[nodiscard]] unsigned get_input_size() const { return num_input_capacity; }
  [[nodiscard]] unsigned get_agg_usage() const {
    return total_agg_buffer_usage;
  }
  [[nodiscard]] unsigned get_edge_usage() const {
    return total_edge_buffer_usage;
  }
  [[nodiscard]] unsigned get_current_output_node_size() const {
    return current_output_node_size;
  }

private:
  std::map<unsigned, output_node> current_window;

  // this size if fixed because it's only decided by the input buffer
  unsigned num_input_capacity;
  // the number of agg buffer being used
  unsigned total_agg_buffer_usage{};
  // the number of edge buffuer being used
  unsigned total_edge_buffer_usage{};
  // number of edges of next input window;
  unsigned current_item_count{};

  unsigned current_output_node_size;
};

// TODO the pull should be ready to send partial
class output_pool {
public:
  explicit output_pool(const Graph &m_graph, bool enable_outer_order = false,
                       std::string outer_order_file_name = "");

  unsigned get_node_total_len(unsigned node_id) const {
    return all_remaining_output_nodes.at(node_id).get_total_len();
  }
  // get a new input line
  [[nodiscard]] output_node get_next_input_line() const;

  std::pair<unsigned, unsigned> get_next_edge() const {
    auto real_position = this->enable_ordered_list
                             ? order_list[current_position]
                             : current_position;

    assert(real_position ==
           all_remaining_output_nodes.at(real_position).get_output_node_id());

    return {real_position, all_remaining_output_nodes.at(real_position).top()};
  }
  std::pair<unsigned, unsigned> get_next_edge_and_move() {
    assert(!enable_ordered_list or
           order_list.size() == all_remaining_output_nodes.size());

    assert(!enable_ordered_list or current_position < order_list.size());
    auto real_position = this->enable_ordered_list
                             ? order_list[current_position]
                             : current_position;

    assert(real_position ==
           all_remaining_output_nodes.at(real_position).get_output_node_id());

    // will finish this node, just move to the next one
    if (all_remaining_output_nodes.at(real_position).get_remaining() == 1) {
      current_position++;
    }
    return {real_position, all_remaining_output_nodes.at(real_position).pop()};
  }
  // deprecated, now use get next_edge and move
  output_node get_next_input_line_and_move();
  [[maybe_unused]] void only_move() { current_position++; }
  void reset() {
    current_position = 0;
    // reset nodes's position
    for (auto &&i : all_remaining_output_nodes) {
      i.reset();
    }
  }

  bool have_next_col() {
    return current_position < all_remaining_output_nodes.size();
  }

private:
  std::vector<output_node> all_remaining_output_nodes;
  std::vector<int> order_list;
  // represent current node id;
  unsigned current_position = 0;
  bool enable_ordered_list = false;
};

} // namespace fast_sched
#endif // GCN_SIM_FAST_SCHED_H
