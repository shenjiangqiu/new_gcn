//
// Created by sjq on 3/26/21.
//

#include "sliding_window_dense.h"
#include "Slide_window.h"
//
// Created by sjq on 1/4/21.
//
#include "boost/range/irange.hpp"
#include "globals.h"
#include "size.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <utility>

// if we still have a part of a chunk left, handle it

using boost::irange;

unsigned dense_window::getX() const { return x; }

std::vector<unsigned int> dense_window::getY() const { return y; }

unsigned dense_window::getXw() const { return xw; }

unsigned dense_window::getLevel() const { return level; }

std::vector<uint64_t> dense_window::getInputAddr() const { return input_addr; }

uint64_t dense_window::getEdgeAddr() const { return edge_addr; }

uint64_t dense_window::getOutputAddr() const { return output_addr; }

unsigned dense_window::getInputLen() const { return input_len; }

unsigned dense_window::getEdgeLen() const { return edge_len; }

unsigned dense_window::getOutputLen() const { return output_len; }

unsigned dense_window::getNumEdgesInWindow() const {
  return num_edges_in_window;
}

unsigned dense_window::getCurrentNodeSize() const { return current_node_size; }

bool dense_window::operator==(const dense_window &rhs) const {
  return x == rhs.x && y == rhs.y && xw == rhs.xw && level == rhs.level &&
         input_addr == rhs.input_addr && edge_addr == rhs.edge_addr &&
         output_addr == rhs.output_addr && input_len == rhs.input_len &&
         edge_len == rhs.edge_len && output_len == rhs.output_len &&
         num_edges_in_window == rhs.num_edges_in_window &&
         current_node_size == rhs.current_node_size;
}

bool dense_window::operator!=(const dense_window &rhs) const {
  return !(rhs == *this);
}

bool dense_window::isTheFinalCol() const { return the_final_col; }

bool dense_window::isTheFinalRow() const { return the_final_row; }

bool dense_window::isTheFirstRow() const { return the_first_row; }

void dense_window::setTheFinalRow(bool theFinalRow) {
  the_final_row = theFinalRow;
}

void dense_window::set_location(unsigned int _x, unsigned int _xw,
                                std::vector<unsigned int> _y,
                                unsigned int _level) {
  this->x = _x;
  this->xw = _xw;
  this->y = std::move(_y);
  this->level = _level;
}
void dense_window::set_addr(std::vector<uint64_t> inputAddr,
                            unsigned int inputLen, uint64_t edgeAddr,
                            unsigned int edgeLen, uint64_t outputAddr,
                            unsigned int outputLen) {
  this->input_addr = std::move(inputAddr);
  this->input_len = inputLen;
  this->edge_addr = edgeAddr;
  this->edge_len = edgeLen;
  this->output_addr = outputAddr;
  this->output_len = outputLen;
}
void dense_window::set_addr(uint64_t inputAddr, unsigned int inputLen,
                            uint64_t edgeAddr, unsigned int edgeLen,
                            uint64_t outputAddr, unsigned int outputLen) {
  // dense_window only support discrete addr
  throw;
  this->input_addr_c = inputAddr;
  this->input_len = inputLen;
  this->edge_addr = edgeAddr;
  this->edge_len = edgeLen;
  this->output_addr = outputAddr;
  this->output_len = outputLen;
}
void dense_window::set_size(unsigned int currentEdges,
                            unsigned int currentNodeSize) {
  this->num_edges_in_window = currentEdges;
  this->current_node_size = currentNodeSize;
}
void dense_window::set_prop(bool _the_final_col, bool theFinalRow,
                            bool theFirstRow, bool theFinalLayer) {
  this->the_final_col = _the_final_col;
  this->the_final_row = theFinalRow;
  this->the_first_row = theFirstRow;
  this->the_final_layer = theFinalLayer;
}
void dense_window::set_location(unsigned int, unsigned int, unsigned int,
                                unsigned int, unsigned int) {
  throw std::runtime_error("dense window do not support this");
}
unsigned int dense_window::getY_c() const {
  throw std::runtime_error("dense window do not support this");
}
uint64_t dense_window::getInputAddr_c() const {
  throw std::runtime_error("dense window do not support this");
}
unsigned int dense_window::getYw() const { return y.size(); }

dense_window_set::dense_window_set(std::shared_ptr<Graph> mGraph,
                                   std::vector<int> xwS, std::vector<int> ywS,
                                   std::vector<int> nodeSizeS, int totalLevel,
                                   bool is_dense)
    : m_graph(std::move(mGraph)), xw_s(std::move(xwS)), yw_s(std::move(ywS)),
      node_size_s(std::move(nodeSizeS)), total_level(totalLevel) {
  // we need to build a multilevel windows. the levels are layer->column->row
  if (totalLevel < 2) {
    throw std::runtime_error("at lease 2 layer needed");
  }
  // make sure the input is currect
  if (xw_s.size() != unsigned(total_level - 1)) {
    throw;
  }
  if (node_size_s.size() != unsigned(total_level)) {
    throw;
  }

  // prepare for each layer
  uint64_t start_addr = 0xff11ff00;

  // for the first layer, we should ignore the empty entries
  node_addrs.push_back(start_addr);
  start_addr += m_graph->get_num_nodes() *
                (node_size_s[0] - config::ignore_neighbor) * single_node_size;
  // for the remaining layer, keep all the entries.
  for (auto l : irange(1, total_level)) {
    node_addrs.push_back(start_addr);
    start_addr += m_graph->get_num_nodes() * node_size_s[l] * single_node_size;
  }

  // each layer
  uint64_t total_len = 0;
  for (auto level_i : irange(0, total_level - 1)) {

    bool the_last_layer = false;
    if (level_i == total_level - 2) {
      the_last_layer = true;
    }

    auto col_i = 0;
    unsigned current_layer_input_len = 0;
    while (col_i < (int)m_graph->get_num_nodes()) {
      unsigned current_col_input_len = 0;

      // this is the all new column, so the previous one must be the first row.
      // the number of edges belong to this row of this column range
      // example:
      //  1. x . . x
      //  2. . . x x
      //  3. x x x x
      //  row_to_count[1]=2
      //  row_to_count[2]=2
      //  row_to_count[3]=4
      std::map<int, int> row_to_count;

      // setup the end of the column bound
      // that's the last element+1
      auto col_end = col_i + xw_s[level_i];
      bool the_final_col = false;
      auto &&edge_index = m_graph->get_edge_index();

      // if the col_end(ptr) point to the last element, it must be the last col
      // of this layer
      if (col_end >= (int)m_graph->get_num_nodes()) {
        col_end = m_graph->get_num_nodes();
        the_final_col = true;
      } else {
        // if the reset of nodes do not have any edges, it's also the last col
        if (edge_index[col_end] == edge_index[m_graph->get_num_nodes()]) {
          the_final_col = true;
        }
      }

      // setup the index to the idx[] from the ptr[]
      // start is the first edge
      auto start_edge_index = edge_index[col_i];

      auto start_edge_addr = m_graph->get_edge_addr(start_edge_index);
      // end is the first edge of the col_end, noticed that, col_end
      // is the last col+1, so end is the last edge+1.
      // if col_end is the last one of ptr[],
      // the end_edge_index will be the end() of the idx[]
      // for example: num_nodes=3,len(ptr)=4,len(idx)=9
      // ptr: 0 3 5 9
      // idx 123 12 *1234*
      // col_start=2,col_end=3
      // start_edge_index=5,end_edge_index=9
      // the including edges: 1,2,3,4
      auto end_edge_index = m_graph->get_edge_index()[col_end];
      // the length(in bytes) of the edge to be read
      auto edge_len = (end_edge_index - start_edge_index) * 4;

      unsigned output_len =
          (col_end - col_i) * node_size_s.at(level_i + 1) * single_node_size;
      uint64_t output_addr =
          node_addrs.at(level_i + 1) +
          col_i * node_size_s.at(level_i + 1) * single_node_size;

      // count the edges for each row.
      for (auto e_index : irange(start_edge_index, end_edge_index)) {
        row_to_count[m_graph->get_edges()[e_index]]++;
      }

      if (is_dense) {
        // indicate the next window is the first row, after build the first
        // window of this column, the_first_row will be set to false.

        auto first_iter = row_to_count.begin();
        // the vector contains at most yw_s[level_i] pointer,
        // which point to a valid row in row_to_count
        // all_edges will compose  a single window
        auto all_edges = std::vector<decltype(first_iter)>();
        all_edges.reserve(yw_s[level_i]);
        // build windows for this col(col_i to col_end)
        // when all rows are included, then break
        // we build the windows according to the row_to_count,
        // it's a ordered map from node id to edge count
        // in dense model, we just need to include those nodes with at least one
        // edge, so just go through the row_to_count is good enough
        // while-loop, build all rows in this column
        while (true) {
          all_edges.clear();
          unsigned current_row_input_len = 0;
          bool the_first_row = first_iter == row_to_count.begin();
          // add at mose yw_s[level_i] nodes into the current window
          for ([[maybe_unused]] auto _ : irange(0, yw_s[level_i])) {
            if (first_iter == row_to_count.end()) {
              break;
            }
            all_edges.push_back(first_iter++);
          }
          // at this point , first_iter become the first iter of the next round

          // no nodes added, means first_iter is end;
          if (all_edges.empty()) {
            break;
          }

          bool the_last_row = first_iter == row_to_count.end();

          // build the window here
          // number edges in this window
          unsigned total_edges = 0;
          // all the nodes in this window
          std::vector<unsigned> input_nodes;
          // all the nodes addrs in this window
          std::vector<uint64_t> input_addrs;

          // a simple function to get the node addr by node id
          auto get_addr_by_node = [&](unsigned node_id) {
            if (level_i == 0) {
              return node_addrs[0] +
                     node_id * (node_size_s[0] - config::ignore_neighbor) *
                         single_node_size;
            } else {
              return node_addrs[level_i] +
                     node_id * node_size_s[level_i] * single_node_size;
            }
          };

          // gain information from all_edges
          for (auto node : all_edges) {
            auto item = *node;
            auto node_id = item.first;
            auto count = item.second;
            total_edges += count;
            input_nodes.push_back(node_id);
            input_addrs.push_back(get_addr_by_node(node_id));
          }

          // start to setup the window
          auto window = std::make_shared<dense_window>();
          window->set_location(col_i, col_end - col_i, input_nodes, level_i);
          window->set_addr(input_addrs, input_addrs.size(), start_edge_addr,
                           edge_len, output_addr, output_len);
          window->set_size(total_edges, node_size_s[level_i]);
          window->set_prop(the_final_col, the_last_row, the_first_row,
                           the_last_layer);

          m_sliding_window_vec.push_back(
              std::static_pointer_cast<sliding_window_interface>(window));

          current_row_input_len =
              input_nodes.size() * node_size_s[level_i] * single_node_size;
          current_col_input_len += current_row_input_len;
        }
      } else {
        // build shrink-skip based window
        // skipping
        unsigned row_i = 0;

        bool the_first_row = true;
        while (row_i < m_graph->get_num_nodes()) {
          // skipping
          // original algo:
          //          while (row_i < m_graph->get_num_nodes() and
          //                 !row_to_count.count(row_i)) {
          //            // empty line
          //            row_i++;
          //          }
          // new:
          // find the next valid key
          auto row_i_iter = row_to_count.lower_bound(row_i);

          if (row_i_iter == row_to_count.end()) {
            break;
          }
          row_i = row_i_iter->first;

          auto row_end = row_i + yw_s[level_i];
          if (row_end > m_graph->get_num_nodes()) {
            row_end = m_graph->get_num_nodes();
          }
          // shrinking
          // TODO, make it more efficient!
          while (!row_to_count.count(row_end - 1)) {
            row_end--;
          }
          // now we build a window;
          assert(row_end > row_i);
          // the point to row_to_count of the row_i
          auto start_iter = row_to_count.lower_bound(row_i);
          // the point to row_to_count of the row_end, if the is no valid nodes
          // after row_end(included), it will point to row_to_count.end()
          auto end_iter = row_to_count.lower_bound(row_end);
          auto total_edges = std::accumulate(
              start_iter, end_iter, 0,
              [](int value, auto &&pair) { return value + pair.second; });
          uint64_t input_addr = 0;
          if (level_i == 0) {
            input_addr = node_addrs.at(level_i) +
                         row_i * (node_size_s.at(0) - config::ignore_neighbor) *
                             single_node_size;
          } else {
            input_addr = node_addrs.at(level_i) +
                         row_i * node_size_s.at(level_i) * single_node_size;
          }

          // the first layer should ignore the empty entries
          unsigned input_len =
              level_i == 0 ? (row_end - row_i) *
                                 (node_size_s.at(0) - config::ignore_neighbor) *
                                 single_node_size
                           : (row_end - row_i) * node_size_s.at(level_i) *
                                 single_node_size;

          current_col_input_len += input_len;

          auto window = std::make_shared<Slide_window>();
          window->set_location(col_i, col_end - col_i, row_i, row_end - row_i,
                               level_i);
          window->set_addr(input_addr, input_len, start_edge_addr, edge_len,
                           output_addr, output_len);
          window->set_size(total_edges, node_size_s[level_i]);
          bool the_last_row = false;
          // to determine the_last_row, we need to test the row_to_count
          //
          if (end_iter == row_to_count.end()) {
            the_last_row = true;
          }
          window->set_prop(the_final_col, the_last_row, the_first_row,
                           the_last_layer);
          the_first_row = false;
          m_sliding_window_vec.push_back(
              std::static_pointer_cast<sliding_window_interface>(window));

          current_col_input_len +=
              input_len * node_size_s[level_i] * single_node_size;

          row_i = row_end;
        }
      }

      current_layer_input_len += current_col_input_len;
      col_i = col_end;
    }
    // std::cout << "\n\nlayer input len: " << current_layer_input_len
    //          << std::endl;
    total_len += current_layer_input_len;
  }
  // std::cout << "\n\ntotal_len: " << total_len << std::endl;
}

std::vector<std::shared_ptr<sliding_window_interface>>::iterator
dense_window_set::begin() {
  return m_sliding_window_vec.begin();
}

std::vector<std::shared_ptr<sliding_window_interface>>::iterator
dense_window_set::end() {
  return m_sliding_window_vec.end();
}
