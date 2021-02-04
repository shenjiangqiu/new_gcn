//
// Created by sjq on 1/4/21.
//

#include "Slide_window.h"

#include "globals.h"
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
                           int currentNodeSize, bool the_final_col,
                           bool theFinalRow, bool theFirstRow,
                           bool theFinalColOfTheLayer)
    : x(x), y(y), xw(xw), yw(yw), level(level), input_addr(inputAddr),
      edge_addr(edgeAddr), output_addr(outputAddr), input_len(inputLen),
      edge_len(edgeLen), output_len(outputLen),
      num_nodes_in_window(numNodesInWindow), current_node_size(currentNodeSize),
      the_final_col(the_final_col),
      the_final_col_of_the_layer(theFinalColOfTheLayer),
      the_final_row(theFinalRow),

      the_first_row(theFirstRow) {}

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
bool Slide_window::isTheFinalCol() const { return the_final_col; }
bool Slide_window::isTheFinalRow() const { return the_final_row; }
bool Slide_window::isTheFirstRow() const { return the_first_row; }
void Slide_window::setTheFinalRow(bool theFinalRow) {
  the_final_row = theFinalRow;
}
bool Slide_window::isTheFinalColOfTheLayer() const {
  return the_final_col_of_the_layer;
}

Slide_window_set::Slide_window_set(std::shared_ptr<Graph> mGraph,
                                   std::vector<int> xwS, std::vector<int> ywS,
                                   std::vector<int> nodeSizeS, int totalLevel)
    : m_graph(std::move(mGraph)), xw_s(std::move(xwS)), yw_s(std::move(ywS)),
      node_size_s(std::move(nodeSizeS)), total_level(totalLevel) {
  assert(xw_s.size() == total_level - 1);
  assert(node_size_s.size() == total_level);
  bool the_first_row;
  uint64_t start_addr = 0xff11ff00;
  // for the first layer, we should ignore the empty entries
  node_addrs.push_back(start_addr);
  start_addr += m_graph->get_num_nodes() *
                (node_size_s[0] - config::ignore_neighbor) * single_node_size;
  // for the remaining layer, keep all the entries.
  for (auto l = 1; l < totalLevel; l++) {
    node_addrs.push_back(start_addr);
    start_addr += m_graph->get_num_nodes() * node_size_s[l] * single_node_size;
  }

  // each layer
  uint64_t total_len = 0;
  for (auto level_i = 0; level_i < totalLevel - 1; level_i++) {
    if (level_i != 0) {
      m_sliding_window_vec.back().setTheFinalRow(true);
      m_sliding_window_multi_level[level_i - 1].back().back().setTheFinalRow(
          true);
    }
    m_sliding_window_multi_level.emplace_back();
    auto col_i = 0;
    uint current_layer_input_len = 0;
    while (col_i < m_graph->get_num_nodes()) {
      if (!m_sliding_window_multi_level[level_i].empty()) {

        m_sliding_window_multi_level[level_i].back().back().setTheFinalRow(
            true);
        m_sliding_window_vec.back().setTheFinalRow(true);
      }
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
      the_first_row = true;
      uint current_col_input_len = 0;
      while (row_i < m_graph->get_num_nodes()) {
        // skipping
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
        // shrinking
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

        uint64_t input_addr = 0;
        if (level_i == 0) {
          input_addr = node_addrs.at(level_i) +
                       row_i * (node_size_s.at(0) - config::ignore_neighbor) *
                           single_node_size;
        } else {
          input_addr = node_addrs.at(level_i) +
                       row_i * node_size_s.at(level_i) * single_node_size;
        }

        uint64_t edge_addr = m_graph->get_edge_addr(edge_index.at(col_i));

        uint64_t output_addr =
            node_addrs.at(level_i + 1) +
            col_i * node_size_s.at(level_i + 1) * single_node_size;

        // the first layer should ignore the empty entries
        int input_len =
            level_i == 0 ? (row_end - row_i) *
                               (node_size_s.at(0) - config::ignore_neighbor) *
                               single_node_size
                         : (row_end - row_i) * node_size_s.at(level_i) *
                               single_node_size;
        current_col_input_len += input_len;
        auto edge_len = (edge_index.at(col_end) - edge_index.at(col_i)) * 4;

        int output_len =
            (col_end - col_i) * node_size_s.at(level_i + 1) * single_node_size;
        bool the_last_col = ((level_i == total_level - 2) and
                             col_end >= m_graph->get_num_nodes());
        bool the_last_col_of_the_layer = col_end >= m_graph->get_num_nodes();
        m_sliding_window_vec.emplace_back(
            col_i, row_i, xw_s[level_i], row_end - row_i, level_i, input_addr,
            edge_addr, output_addr, input_len, edge_len, output_len, total_node,
            node_size_s[level_i], the_last_col, false, the_first_row,
            the_last_col_of_the_layer);
        m_sliding_window_multi_level.back().back().emplace_back(
            col_i, row_i, xw_s[level_i], row_end - row_i, level_i, input_addr,
            edge_addr, output_addr, input_len, edge_len, output_len, total_node,
            node_size_s[level_i], the_last_col, false, the_first_row,
            the_last_col_of_the_layer);
        row_i = row_end;
        the_first_row = false;
      }
      //std::cout << "col input len: " << current_col_input_len << std::endl;
      current_layer_input_len += current_col_input_len;
      col_i = col_end;
    }
    //std::cout << "\n\nlayer input len: " << current_layer_input_len
    //          << std::endl;
    total_len += current_layer_input_len;
  }
  //std::cout << "\n\ntotal_len: " << total_len << std::endl;
  m_sliding_window_multi_level.back().back().back().setTheFinalRow(true);
  m_sliding_window_vec.back().setTheFinalRow(true);
}
slide_window_set_iterator Slide_window_set::begin() {
  for (auto i = m_sliding_window_multi_level.begin();
       i < m_sliding_window_multi_level.end(); i++) {
    for (auto j = i->begin(); j < i->end(); i++) {
      for (auto k = j->begin(); k < j->end(); j++) {
        return slide_window_set_iterator(k, j, i,
                                         m_sliding_window_multi_level.end());
      }
    }
  }
  throw std::runtime_error("cannot find a valid entry for begin");
}
slide_window_set_iterator Slide_window_set::end() {
  return begin().setThirdIter(m_sliding_window_multi_level.end());
}
bool slide_window_set_iterator::have_next_row() {
  return !((*this)->isTheFinalCol() and (*this)->isTheFinalRow());
}
slide_window_set_iterator::slide_window_set_iterator(
    const std::vector<Slide_window>::iterator &firstIter,
    const std::vector<std::vector<Slide_window>>::iterator &secondIter,
    const std::vector<std::vector<std::vector<Slide_window>>>::iterator
        &thirdIter,
    std::vector<std::vector<std::vector<Slide_window>>>::iterator thirdEnd)
    : first_iter(firstIter), second_iter(secondIter), third_iter(thirdIter),
      third_end(thirdEnd) {}
slide_window_set_iterator &slide_window_set_iterator::operator--() {
  throw std::runtime_error("not implemented yet");
  while (third_iter != third_end) {
    while (second_iter != third_iter->end()) {
      while (first_iter != second_iter->end()) {
        first_iter++;
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
      second_iter++;
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
    third_iter++;
    if (third_iter != third_end) {
      second_iter = third_iter->begin();
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();

        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
  }
  return *this;
}
slide_window_set_iterator &slide_window_set_iterator::operator--(int) {
  throw std::runtime_error("not implemented yet");
  while (third_iter != third_end) {
    while (second_iter != third_iter->end()) {
      while (first_iter != second_iter->end()) {
        first_iter++;
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
      second_iter++;
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
    third_iter++;
    if (third_iter != third_end) {
      second_iter = third_iter->begin();
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();

        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
  }
  return *this;
}
slide_window_set_iterator &slide_window_set_iterator::operator++() {
  if (third_iter == third_end) {
    return *this;
  }
  while (third_iter != third_end) {
    while (second_iter != third_iter->end()) {
      while (first_iter != second_iter->end()) {
        first_iter++;
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
      second_iter++;
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();
        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
    third_iter++;
    if (third_iter != third_end) {
      second_iter = third_iter->begin();
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();

        if (first_iter != second_iter->end()) {
          return *this;
        }
      }
    }
  }
  return *this;
}
slide_window_set_iterator slide_window_set_iterator::operator++(int) {
  auto copy = *this;
  if (third_iter == third_end) {
    return *this;
  }
  while (third_iter != third_end) {
    while (second_iter != third_iter->end()) {
      while (first_iter != second_iter->end()) {
        first_iter++;
        if (first_iter != second_iter->end()) {
          return copy;
        }
      }
      second_iter++;
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();
        if (first_iter != second_iter->end()) {
          return copy;
        }
      }
    }
    third_iter++;
    if (third_iter != third_end) {
      second_iter = third_iter->begin();
      if (second_iter != third_iter->end()) {
        first_iter = second_iter->begin();

        if (first_iter != second_iter->end()) {
          return copy;
        }
      }
    }
  }
  return copy;
}
bool slide_window_set_iterator::have_next_col() {
  auto next_col = std::next(second_iter);
  if (next_col == third_iter->end()) {
    auto next_level = std::next(third_iter);
    if (next_level == third_end or next_level->begin() == next_level->end()) {
      return false;
    }
    return true;

  } else {
    return true;
  }
}
slide_window_set_iterator slide_window_set_iterator::get_next_col() {
  auto next_col = std::next(second_iter);
  if (next_col == third_iter->end()) {
    auto next_level = std::next(third_iter);
    if (next_level == third_end or next_level->begin() == next_level->end()) {
      throw std::runtime_error("no next col,should check before get");
    }
    return slide_window_set_iterator(next_level->begin()->begin(),
                                     next_level->begin(), next_level,
                                     third_end);

  } else {
    return slide_window_set_iterator(next_col->begin(), next_col, third_iter,
                                     third_end);
  }
}
Slide_window &slide_window_set_iterator::operator*() {
  if (third_iter == third_end) {
    throw std::runtime_error("out of bound");
  }
  return *first_iter;
}
slide_window_set_iterator &slide_window_set_iterator::setThirdEnd(
    const std::vector<std::vector<std::vector<Slide_window>>>::iterator
        &thirdEnd) {
  third_end = thirdEnd;
  return *this;
}
bool slide_window_set_iterator::operator==(
    const slide_window_set_iterator &rhs) const {
  if (third_iter == third_end and rhs.third_iter == rhs.third_end) {
    return true;
  }
  return first_iter == rhs.first_iter && second_iter == rhs.second_iter &&
         third_iter == rhs.third_iter && third_end == rhs.third_end;
}
bool slide_window_set_iterator::operator!=(
    const slide_window_set_iterator &rhs) const {
  return !(rhs == *this);
}
slide_window_set_iterator slide_window_set_iterator::setThirdIter(
    const std::vector<std::vector<std::vector<Slide_window>>>::iterator
        &thirdIter) {
  third_iter = thirdIter;
  return *this;
}
const Slide_window &slide_window_set_iterator::operator*() const {
  return *first_iter;
}
