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
  bool operator==(const Slide_window &rhs) const;

  bool operator!=(const Slide_window &rhs) const;

  Slide_window() = default;

  Slide_window(int x, int y, int xw, int yw, int level, uint64_t inputAddr,
               uint64_t edgeAddr, uint64_t outputAddr, int inputLen,
               int edgeLen, int outputLen, int numNodesInWindow,
               int currentNodeSize, bool the_final_col, bool theFinalRow,
               bool theFirstRow, bool theFinalColOfTheLayer);

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
  bool isTheFinalCol() const;
  bool isTheFinalRow() const;
  bool isTheFirstRow() const;
  void setTheFinalRow(bool theFinalRow);

private:
  int x, y, xw, yw, level;
  uint64_t input_addr, edge_addr, output_addr;
  int input_len, edge_len, output_len;
  int num_nodes_in_window;
  int current_node_size;
  bool the_final_col;
  bool the_final_col_of_the_layer;

public:
  bool isTheFinalColOfTheLayer() const;
private:
  bool the_final_row;
  bool the_first_row;

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
        simple
            ? format_to(ctx.out(), "{} {} {} {} {} fnc:{} fnr:{} fsr:{}",
                        p.getX(), p.getY(), p.getXw(), p.getYw(), p.getLevel(),
                        p.isTheFinalCol(), p.isTheFinalRow(), p.isTheFirstRow())
            : format_to(ctx.out(),
                        "x {} y {} {} {} l {} i_a {} e_a {} o_a {} i_l {} "
                        "e_l {} o_l {} n_n:{} fnc:{} fnr:{} fsr:{}",
                        p.getX(), p.getY(), p.getXw(), p.getYw(), p.getLevel(),
                        p.getInputAddr(), p.getEdgeAddr(), p.getOutputAddr(),
                        p.getInputLen(), p.getEdgeLen(), p.getOutputLen(),
                        p.getNumNodesInWindow(), p.isTheFinalCol(),
                        p.isTheFinalRow(), p.isTheFirstRow());
    return out;
  }
};

class slide_window_set_iterator
    : public std::iterator<std::bidirectional_iterator_tag, Slide_window> {
public:
  bool have_next_row();
  slide_window_set_iterator(
      const std::vector<Slide_window>::iterator &firstIter,
      const std::vector<std::vector<Slide_window>>::iterator &secondIter,
      const std::vector<std::vector<std::vector<Slide_window>>>::iterator
          &thirdIter,
      std::vector<std::vector<std::vector<Slide_window>>>::iterator thirdEnd);
  slide_window_set_iterator() {}

  slide_window_set_iterator &operator--();
  slide_window_set_iterator &operator--(int);
  slide_window_set_iterator &operator++();

  slide_window_set_iterator operator++(int);

  bool have_next_col();
  bool is_last_row() { return std::next(first_iter) == second_iter->end(); }

  slide_window_set_iterator get_next_col();

  Slide_window &operator*();
  const Slide_window &operator*() const;

  std::vector<Slide_window>::iterator operator->() { return first_iter; }

  slide_window_set_iterator &setThirdEnd(
      const std::vector<std::vector<std::vector<Slide_window>>>::iterator
          &thirdEnd);

  // if both is end, then return true
  // if not end, then need to be all equal.
  bool operator==(const slide_window_set_iterator &rhs) const;

  bool operator!=(const slide_window_set_iterator &rhs) const;

  slide_window_set_iterator setThirdIter(
      const std::vector<std::vector<std::vector<Slide_window>>>::iterator
          &thirdIter);

private:
  std::vector<Slide_window>::iterator first_iter;
  std::vector<std::vector<Slide_window>>::iterator second_iter;
  std::vector<std::vector<std::vector<Slide_window>>>::iterator third_iter;

  std::vector<std::vector<std::vector<Slide_window>>>::iterator third_end;
};

class Slide_window_set {
public:
  Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS,
                   std::vector<int> ywS, std::vector<int> nodeSizeS,
                   int totalLevel);

  const std::vector<Slide_window> &get_windows() {
    return m_sliding_window_vec;
  }

  slide_window_set_iterator begin();

  slide_window_set_iterator end();

private:
  std::shared_ptr<Graph> m_graph;
  std::vector<int> xw_s;
  std::vector<int> yw_s;

  std::vector<int> node_size_s;

  int total_level;

  std::vector<Slide_window> m_sliding_window_vec;
  // 3 dimension vector: level-col-row-element
  // m_sliding_window_multi_level[1][2][3],layer,1,col,2,row3
  std::vector<std::vector<std::vector<Slide_window>>>
      m_sliding_window_multi_level;

  std::vector<uint64_t> node_addrs; // for each level;
};

#endif // GCN_SIM_SLIDE_WINDOW_H
