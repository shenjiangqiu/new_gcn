//
// Created by sjq on 1/4/21.
//

#ifndef GCN_SIM_SLIDE_WINDOW_H
#define GCN_SIM_SLIDE_WINDOW_H

#include "fmt/format.h"
#include <algorithm>
#include <graph.h>
#include <iostream>
#include <memory>

class Slide_window {
public:
  Slide_window() = default;

  Slide_window(int x, int y, int xw, int yw, int level, uint64_t inputAddr,
               uint64_t edgeAddr, uint64_t outputAddr, int inputLen,
               int edgeLen, int outputLen, int numNodesInWindow,
               int currentNodeSize);

  [[nodiscard]] int getX() const;

  [[nodiscard]] int getY() const;

  [[nodiscard]] int getXw() const;

  [[nodiscard]] int getYw() const;

  [[nodiscard]] int getLevel() const;

  [[nodiscard]] uint64_t getInputAddr() const;

  [[nodiscard]] uint64_t getEdgeAddr() const;

  [[nodiscard]] uint64_t getOutputAddr() const;

  [[nodiscard]] int getInputLen() const;

  [[nodiscard]] int getEdgeLen() const;

  [[nodiscard]] int getOutputLen() const;

  [[nodiscard]] int getCurrentNodeSize() const;

private:
  int x, y, xw, yw, level;
  uint64_t input_addr, edge_addr, output_addr;
  int input_len, edge_len, output_len;
  int num_nodes_in_window;
  int current_node_size;

public:
  [[nodiscard]] int getNumNodesInWindow() const;
};

template <> struct fmt::formatter<Slide_window> {
  bool simple = false;

  constexpr auto parse(format_parse_context &ctx) {

    // Parse the presentation format and store it in the formatter:
    auto it = ctx.begin();
    while (it != ctx.end() and *it == 's') {
      if (*it == 's') {
        simple = true;
      }
      it++;
    }
    if (it != ctx.end() && *it != '}') {
      throw format_error("invalid format");
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const Slide_window &p, FormatContext &ctx) {
    // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) //
    // c++11 ctx.out() is an output iterator to write to.
    auto out =
        simple ? format_to(ctx.out(), "{} {} {} {} {}", p.getX(), p.getY(),
                           p.getXw(), p.getYw(), p.getLevel())
               : format_to(ctx.out(),
                           "x {} y {} {} {} l {} i_a {} e_a {} o_a {} i_l {} "
                           "e_l {} o_l {} n_n {}",
                           p.getX(), p.getY(), p.getXw(), p.getYw(),
                           p.getLevel(), p.getInputAddr(), p.getEdgeAddr(),
                           p.getOutputAddr(), p.getInputLen(), p.getEdgeLen(),
                           p.getOutputLen(), p.getNumNodesInWindow());
    return out;
  }
};

template <typename T>
class slide_window_set_iterator
    : public std::iterator<std::bidirectional_iterator_tag, T> {



};
class Slide_window_set {
public:
  Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS,
                   std::vector<int> ywS, std::vector<int> nodeSizeS,
                   int totalLevel);

  const std::vector<Slide_window> &get_windows() {
    return m_sliding_window_vec;
  }

private:
  std::shared_ptr<Graph> m_graph;
  std::vector<int> xw_s;
  std::vector<int> yw_s;

  std::vector<int> node_size_s;

  int total_level;

  std::vector<Slide_window> m_sliding_window_vec;
  std::vector<std::vector<std::vector<Slide_window>>>
      m_sliding_window_multi_level;

  std::vector<uint64_t> node_addrs; // for each level;
};

#endif // GCN_SIM_SLIDE_WINDOW_H
