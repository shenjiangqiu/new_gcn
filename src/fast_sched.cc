//
// Created by Jiangqiu shen on 5/21/21.
//

#include "fast_sched.h"

#include "cstdint"
#include "limits"
#include <utility>
using namespace fast_sched;
template <typename C> bool all_false(C &c) {
  return std::all_of(c.begin(), c.end(), [](auto v) { return v == false; });
}

output_pool::output_pool(const Graph &m_graph) {
  const auto &ptr = m_graph.get_edge_index();
  const auto &idx = m_graph.get_edges();
  auto num_node = m_graph.get_num_nodes();
  // for each nodes
  for (auto i = 0u; i < num_node; i++) {
    // this is a vertical line
    // build a vertical line
    std::vector<unsigned> input_nodes;
    auto start = ptr[i];
    auto end = ptr[i + 1];
    for (auto j = start; j < end; j++) {
      auto input_node = idx[j];
      input_nodes.push_back(input_node);
    }
    auto out_nd = output_node(input_nodes);
    this->all_remaining_output_nodes.push_back(out_nd);
  }
}
output_node output_pool::get_next_input_line() {
  return this->all_remaining_output_nodes[current_position++];
}

output_node::output_node(const std::vector<unsigned int> &_input_nodes) {
  for (auto &&i : _input_nodes) {
    input_nodes.insert(i);
  }
  not_processed_nodes = input_nodes;
}

const std::set<unsigned int> &output_node::getInputNodes() const {
  return input_nodes;
}

void output_node::setInputNodes(const std::set<unsigned int> &inputNodes) {
  input_nodes = inputNodes;
}

output_node::output_node(std::set<unsigned int> _input_nodes)
    : input_nodes(std::move(_input_nodes)) {
  not_processed_nodes = input_nodes;
}
std::vector<unsigned> output_node::get_next_n_input(unsigned int n) const {
  std::vector<unsigned> out;
  auto front = not_processed_nodes.cbegin();

  for (unsigned i = 0; i < n; i++) {

    if (front != not_processed_nodes.cend()) {
      out.push_back(*front);
      // not_processed_nodes.erase(front);
      front++;
    } else {
      break;
    }
  }

  return out;
}

unsigned output_node::invalid_input(const std::vector<unsigned int> &input) {
  unsigned invalid_items = 0;
  for (auto &&i : input) {
    if (not_processed_nodes.count(i)) {
      not_processed_nodes.erase(i);
      invalid_items++;
    }
  }
  return invalid_items;
}

void current_working_window::invalid(unsigned int id) {
  this->current_valid[id] = false;
  all_finished_col.insert(id);
}
void current_working_window::add(unsigned id, const output_node &nd) {
  this->current_valid[id] = true;
  this->current_window[id] = nd;
  if (all_finished_col.count(id)) {
    all_finished_col.erase(id);
  }
}

void current_working_window::invalid_and_add(unsigned int id,
                                             const output_node &nd) {
  current_valid[id] = true;
  current_window[id] = nd;
  if (all_finished_col.count(id)) {
    all_finished_col.erase(id);
  }
}

// according to each output nd, choose the smallest node
std::vector<unsigned> current_working_window::get_next_input_nodes() {
  // fix bug here, if the selected node reture a vecotor that is smaller than
  // sz, then choose the next one

  auto remainings = num_input_capacity;
  std::vector<unsigned> all_input;

  // the number of edges returned in the input window
  unsigned item_count = 0;

  // in this while loop, we will try to find next sz input nodes to be loaded
  // into the input buffer
  while (remainings > 0) {

    // first step, find a node with fewest input edges.
    auto smallest = UINT_MAX;
    auto selected = 0u;
    // select the smalllest col
    for (auto i = 0u; i < sz; i++) {
      if (this->current_valid[i]) {
        auto &current_node = current_window[i];
        auto size = current_node.get_remaining();
        if (size < smallest) {
          smallest = size;
          selected = i;
        }
      }
    }

    // get the next input window from the selected node
    auto &selected_window = current_window[selected];
    auto next_input = selected_window.get_next_n_input(remainings);
    if (next_input.empty()) {
      // we didn't select a valid node, all node are invalid now;
      assert(all_false(current_valid));
      break;
    }
    // mark all elements in this vector invalid in all other working set, this
    // step prevent choose redundent input nodes
    for (auto i = 0u; i < sz; i++) {
      if (this->current_valid[i]) {
        if (current_window[i].is_all_processed()) {
          current_valid[i] = false;
          all_finished_col.insert(i);
        } else {
          // valid and not all processed
          item_count += current_window[i].invalid_input(next_input);
          if (current_window[i].is_all_processed()) {
            all_finished_col.insert(i);
            current_valid[i] = false;
          }
        }
      }
    }

    all_input.insert(all_input.end(), next_input.begin(), next_input.end());

    // the size of the next input means number of nodes we chooses in this nodes
    remainings -= next_input.size();
  }

  current_item_count = item_count;

  return all_input;
}

current_working_window::current_working_window(unsigned int size,
                                               unsigned int numInputCapacity)
    : sz(size), current_window(size), current_valid(size, false),
      num_input_capacity(numInputCapacity) {
  for (auto i = 0u; i < sz; i++) {
    all_finished_col.insert(i);
  }
}
const std::set<unsigned int> current_working_window::getAllFinishedCol() const {
  return all_finished_col;
}
