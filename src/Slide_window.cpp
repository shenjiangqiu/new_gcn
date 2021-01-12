//
// Created by sjq on 1/4/21.
//

#include "Slide_window.h"

#include "size.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <utility>

Slide_window::Slide_window(int x, int y, int xw, int yw, int level,
                           uint64_t inputAddr, uint64_t edgeAddr,
                           uint64_t outputAddr, int inputLen, int edgeLen,
                           int outputLen, int numNodesInWindow,
                           int currentNodeSize)
    : x(x), y(y), xw(xw), yw(yw), level(level), input_addr(inputAddr),
      edge_addr(edgeAddr), output_addr(outputAddr), input_len(inputLen),
      edge_len(edgeLen), output_len(outputLen),
      num_nodes_in_window(numNodesInWindow),
      current_node_size(currentNodeSize) {}

int Slide_window::getX() const { return x; }

int Slide_window::getY() const { return y; }

int Slide_window::getXw() const { return xw; }

int Slide_window::getYw() const { return yw; }

int Slide_window::getLevel() const { return level; }

uint64_t Slide_window::getInputAddr() const { return input_addr; }

uint64_t Slide_window::getEdgeAddr() const { return edge_addr; }

uint64_t Slide_window::getOutputAddr() const { return output_addr; }

int Slide_window::getInputLen() const { return input_len; }

int Slide_window::getEdgeLen() const { return edge_len; }

int Slide_window::getOutputLen() const { return output_len; }

int Slide_window::getNumNodesInWindow() const { return num_nodes_in_window; }

int Slide_window::getCurrentNodeSize() const { return current_node_size; }
bool Slide_window::operator==(const Slide_window &rhs) const {
  return x == rhs.x && y == rhs.y && xw == rhs.xw && yw == rhs.yw &&
         level == rhs.level && input_addr == rhs.input_addr &&
         edge_addr == rhs.edge_addr && output_addr == rhs.output_addr &&
         input_len == rhs.input_len && edge_len == rhs.edge_len &&
         output_len == rhs.output_len &&
         num_nodes_in_window == rhs.num_nodes_in_window &&
         current_node_size == rhs.current_node_size;
}
bool Slide_window::operator!=(const Slide_window &rhs) const {
  return !(rhs == *this);
}

Slide_window_set::Slide_window_set(std::shared_ptr<Graph> mGraph,
                                   std::vector<int> xwS, std::vector<int> ywS,
                                   std::vector<int> nodeSizeS, int totalLevel)
    : m_graph(std::move(mGraph)), xw_s(std::move(xwS)), yw_s(std::move(ywS)),
      node_size_s(std::move(nodeSizeS)), total_level(totalLevel) {
  assert(xw_s.size() == total_level - 1);
  assert(node_size_s.size() == total_level);

  uint64_t start_addr = 0xff11ff00;
  for (auto l = 0; l < totalLevel; l++) {
    node_addrs.push_back(start_addr);
    start_addr += m_graph->get_num_nodes() * node_size_s[l] * single_node_size;
  }
  //each layer
  for (auto level_i = 0; level_i < totalLevel - 1; level_i++) {
    m_sliding_window_multi_level.emplace_back();
    auto col_i = 0;
    while (col_i < m_graph->get_num_nodes()) {
      m_sliding_window_multi_level[level_i].emplace_back();
      std::map<int, int> row_to_count;
      auto col_end = col_i + xw_s[level_i];
      if (col_end > m_graph->get_num_nodes()) {
        col_end = m_graph->get_num_nodes();
      }
      auto start_edge_index = m_graph->get_edge_index()[col_i];
      auto end_edge_index = m_graph->get_edge_index()[col_end];
      for (auto e_index = start_edge_index; e_index < end_edge_index;
           e_index++) {
        row_to_count[m_graph->get_edges()[e_index]]++;
      }
      auto row_i = 0;
      while (row_i < m_graph->get_num_nodes()) {
        //skipping
        while (row_i < m_graph->get_num_nodes() and
               !row_to_count.count(row_i)) {
          // empty line
          row_i++;
        }
        if (row_i >= m_graph->get_num_nodes()) {
          break;
        }
        auto row_end = row_i + yw_s[level_i];
        if (row_end > m_graph->get_num_nodes()) {
          row_end = m_graph->get_num_nodes();
        }
        //shrinking
        while (!row_to_count.count(row_end - 1)) {
          row_end--;
        }
        // now we build a window;
        assert(row_end > row_i);
        auto lower_bound = row_to_count.lower_bound(row_i);
        auto upper_bound = row_to_count.lower_bound(row_end);
        auto total_node = std::accumulate(
            lower_bound, upper_bound, 0,
            [](int value, auto &&pair) { return value + pair.second; });
        auto &&edge_index = m_graph->get_edge_index();

        uint64_t input_addr =
            node_addrs.at(level_i) +
            row_i * node_size_s.at(level_i) * single_node_size;

        uint64_t edge_addr = m_graph->get_edge_addr(edge_index.at(col_i));

        uint64_t output_addr =
            node_addrs.at(level_i + 1) +
            col_i * node_size_s.at(level_i + 1) * single_node_size;
        int input_len =
            (row_end - row_i) * node_size_s.at(level_i) * single_node_size;

        auto edge_len = (edge_index.at(col_end) - edge_index.at(col_i)) * 4;

        int output_len =
            (col_end - col_i) * node_size_s.at(level_i + 1) * single_node_size;

        m_sliding_window_vec.emplace_back(
            col_i, row_i, xw_s[level_i], row_end - row_i, level_i, input_addr,
            edge_addr, output_addr, input_len, edge_len, output_len, total_node,
            node_size_s[level_i]);
        m_sliding_window_multi_level.back().back().emplace_back(
            col_i, row_i, xw_s[level_i], row_end - row_i, level_i, input_addr,
            edge_addr, output_addr, input_len, edge_len, output_len, total_node,
            node_size_s[level_i]);
        row_i = row_end;
      }
      col_i = col_end;
    }
  }
}
