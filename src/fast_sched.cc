//
// Created by Jiangqiu shen on 5/21/21.
//

#include "fast_sched.h"
#include "limits"
#include <debug_helper.h>
#include <globals.h>
#include <spdlog/spdlog.h>
using namespace fast_sched;
template <typename C> bool all_false(C &c) {
  return std::all_of(c.begin(), c.end(), [](auto v) { return v == false; });
}

output_pool::output_pool(const Graph &m_graph, bool enable_outer_order,
                         std::string outer_order_file_name) {
  const auto &ptr = m_graph.get_edge_index();
  const auto &idx = m_graph.get_edges();
  auto num_node = m_graph.get_num_nodes();
  this->enable_ordered_list = enable_outer_order;
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
    auto out_nd = output_node(input_nodes, i);
    this->all_remaining_output_nodes.push_back(out_nd);
  }

  if (enable_outer_order) {
    fmt::print("read from list name: {} \n", outer_order_file_name);
    auto in = std::ifstream(outer_order_file_name);
    std::string buffer;
    while (in >> buffer) {
      this->order_list.push_back(std::stoi(buffer));
    }
  }
}
output_node output_pool::get_next_input_line() const {
  return this->all_remaining_output_nodes.at(current_position);
}
output_node output_pool::get_next_input_line_and_move() {
  return this->all_remaining_output_nodes[current_position++];
}

output_node::output_node(const std::vector<unsigned int> &_input_nodes,
                         unsigned id)
    : output_node_id(id), not_processed_nodes(_input_nodes) {}

// [[maybe_unused]] const std::set<unsigned int> &
// output_node::getInputNodes() const {
//   return input_nodes;
// }

// [[maybe_unused]] void
// output_node::setInputNodes(const std::set<unsigned int> &inputNodes) {
//   input_nodes = inputNodes;
// }

// output_node::output_node(std::set<unsigned int> _input_nodes, unsigned id,
//                          unsigned edge_size)
//     : input_nodes(std::move(_input_nodes)), output_node_id(id),
//       edgesize(edge_size), not_processed_nodes(input_nodes) {}

// std::vector<unsigned> output_node::get_next_n_input(unsigned int n) const {
//   std::vector<unsigned> out;
//   auto front = not_processed_nodes.cbegin();

//   for (unsigned i = 0; i < n; i++) {

//     if (front != not_processed_nodes.cend()) {
//       out.push_back(*front);
//       // not_processed_nodes.erase(front);
//       front++;
//     } else {
//       break;
//     }
//   }

//   return out;
// }

// unsigned output_node::invalid_input(const std::vector<unsigned int> &input) {
//   unsigned invalid_items = 0;
//   for (auto &&i : input) {
//     if (not_processed_nodes.count(i)) {
//       not_processed_nodes.erase(i);
//       invalid_items++;
//     }
//   }
//   return invalid_items;
// }
// find bug here, not release the badge!!!
// bool fast_sched::current_working_window::can_add(
//     const fast_sched::output_node &node) const {
//   assert((total_edge_buffer_usage == 0 and total_agg_buffer_usage == 0) or
//          (total_edge_buffer_usage != 0 and total_agg_buffer_usage != 0));
//   if (((total_edge_buffer_usage + node.get_edge_size()) <
//        (unsigned)config::edgeSize) and
//       ((total_agg_buffer_usage + current_output_node_size) <
//        (unsigned)config::aggSize)) {
//     return true;
//   } else {
//     return false;
//   }
// // }
// void current_working_window::add(const output_node &nd) {
//   assert(can_add(nd));
//   assert(current_window.count(nd.get_output_node_id()) == 0);
//   current_window.insert({nd.get_output_node_id(), nd});

//   total_edge_buffer_usage += nd.get_edge_size();
//   total_agg_buffer_usage += current_output_node_size;

//   assert(total_edge_buffer_usage <= (unsigned)config::edgeSize);
//   assert(total_agg_buffer_usage <= (unsigned)config::aggSize);

//   spdlog::trace("edge_buffer_ocupy: {} of_total {}", total_edge_buffer_usage,
//                 (int)config::edgeSize);
//   spdlog::trace("agg_buffer_ocupy: {} of_total {}", total_agg_buffer_usage,
//                 (int)config::aggSize);
// }

// // according to each output nd, choose the smallest node
// std::vector<unsigned> current_working_window::get_next_input_nodes() {
//   // fix bug here, if the selected node reture a vecotor that is smaller than
//   // sz, then choose the next one

//   auto remainings = num_input_capacity;
//   std::vector<unsigned> all_input;

//   // the number of edges returned in the input window
//   unsigned item_count = 0;
//   assert(!current_window.empty());
//   // in this while loop, we will try to find next sz input nodes to be loaded
//   // into the input buffer
//   while (remainings > 0) {

//     // first step, find a node with fewest input edges.
//     auto smallest = UINT_MAX;
//     auto selected = current_window.begin();
//     if (selected == current_window.end()) {
//       // no one in window
//       break;
//     }
//     // select the smalllest col
//     if (config::enable_global_sorted_graph) {
//       // just use the bgin
//     } else {
//       for (auto i = current_window.begin(); i != current_window.end(); i++) {

//         auto &current_node = *i;
//         auto size = current_node.second.get_remaining();
//         if (size < smallest) {
//           smallest = size;
//           selected = i;
//         }
//       }
//     }

//     // get the next input window from the selected node
//     auto &selected_window = *selected;
//     auto next_input = selected_window.second.get_next_n_input(remainings);

//     assert(!next_input.empty());
//     // for histogram stats, record the number of output for each input

//     for (auto i : next_input) {
//       auto count = 0u;
//       for (auto &j : current_window) {
//         // count the number of occurence of this input
//         if (j.second.exists(i))
//           count++;
//       }
//       global_definitions.number_to_count_map_for_query[count]++;
//     }

//     // mark all elements in this vector invalid in all other working set,
//     this
//     // step prevent choose redundent input nodes
//     for (auto &i : current_window) {

//       // valid and not all processed
//       item_count += i.second.invalid_input(next_input);
//     }
//     // remove all empty entry, which means

//     // FIXME , fix bug here, when delete a line, should release the entry
//     // To release the entry,1. need to release the agg and edge buffer
//     // 2. need to invalid all hashtable(make sure!!!)
//     // what about cycle in hashtable??

//     auto i = current_window.begin();
//     while (i != current_window.end()) {
//       if (i->second.is_all_processed()) {
//         // fix the bug that will cause edge buufer not return to zero.
//         total_edge_buffer_usage -= i->second.get_edge_size();
//         total_agg_buffer_usage -= current_output_node_size;
//         i = current_window.erase(i);

//       } else {
//         i++;
//       }
//     }

//     all_input.insert(all_input.end(), next_input.begin(), next_input.end());

//     // the size of the next input means number of nodes we chooses in this
//     nodes remainings -= next_input.size();
//   }

//   current_item_count = item_count;

//   return all_input;
// }
// // output node_size, single node size in Byte
current_working_window::current_working_window(unsigned int numInputCapacity,
                                               unsigned output_node_size)
    : num_input_capacity(numInputCapacity),
      current_output_node_size(output_node_size) {}
