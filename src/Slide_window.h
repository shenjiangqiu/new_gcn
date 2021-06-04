//
// Created by sjq on 1/4/21.
//

#ifndef GCN_SIM_SLIDE_WINDOW_H_OLD
#define GCN_SIM_SLIDE_WINDOW_H_OLD

#include "fmt/format.h"
#include "sliding_window_interface.h"
#include <algorithm>
#include <graph.h>
#include <iostream>
#include <memory>
class Slide_window : public sliding_window_interface {
public:

  void set_location(unsigned int x, unsigned int _xw, unsigned int _y,
                    unsigned int yw, unsigned int _level) override;

  void set_location(unsigned int x, unsigned int _xw,
                    std::vector<unsigned int> _y, unsigned int _level) override;
  void set_addr(std::vector<uint64_t> inputAddr, unsigned int inputLen,
                uint64_t edgeAddr, unsigned int edgeLen, uint64_t outputAddr,
                unsigned int outputLen) override;
  void set_addr(uint64_t inputAddr, unsigned int inputLen, uint64_t edgeAddr,
                unsigned int edgeLen, uint64_t outputAddr,
                unsigned int outputLen) override;
  void set_size(unsigned int currentEdges,
                unsigned int currentnodeDim) override;
  void set_prop(bool theFinalCol, bool theFinalRow, bool theFirstRow,
                bool theFinalLayer) override;
  unsigned int getY_c() const override;
  uint64_t getInputAddr_c() const override;
  bool isTheFinalLayer() const override;

public:
  bool operator==(const Slide_window &rhs) const;

  bool operator!=(const Slide_window &rhs) const;

  [[nodiscard]] unsigned getX() const override;

  [[nodiscard]] std::vector<unsigned> getY() const override;

  [[nodiscard]] unsigned getXw() const override;

  [[nodiscard]] unsigned getYw() const override;

  [[nodiscard]] unsigned getLevel() const override;

  [[nodiscard]] std::vector<uint64_t> getInputAddr() const override;

  [[nodiscard]] uint64_t getEdgeAddr() const override;

  [[nodiscard]] uint64_t getOutputAddr() const override;

  [[nodiscard]] unsigned getInputLen() const override;

  [[nodiscard]] unsigned getEdgeLen() const override;

  [[nodiscard]] unsigned getOutputLen() const override;

  [[nodiscard]] unsigned getCurrentnodeDim() const override;
  [[nodiscard]] bool isTheFinalCol() const override;
  [[nodiscard]] bool isTheFinalRow() const override;
  [[nodiscard]] bool isTheFirstRow() const override;
  void setTheFinalRow(bool theFinalRow) override;

  bool isTheFinalColOfTheLayer() const;

  [[nodiscard]] unsigned getNumEdgesInWindow() const override;

private:
  unsigned x{};  // x: starting vertex ID in the aggregation buffer
  unsigned y{};  // y: starting vertex ID in the input buffer
  unsigned xw{}; // cnt of vertices in the the aggregation buffer
  unsigned yw{}; // cnt of vertices in the the input buffer
  unsigned level{};
  uint64_t input_addr_c{}, edge_addr{}, output_addr{};
  unsigned input_len{}, edge_len{}, output_len{}; // in unit of bytes.
  unsigned num_edges_in_window{};                 //#edges in the window
  unsigned current_node_dim{};                   // A feature dim.
  bool the_final_col{};
  bool the_final_row{};
  bool the_first_row{};
  bool the_final_layer{};

  // the number of nodes that have edges connected
  unsigned valid_nodes{};
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
                        p.getX(), p.getY_c(), p.getXw(), p.getYw(), p.getLevel(),
                        p.isTheFinalCol(), p.isTheFinalRow(), p.isTheFirstRow())
            : format_to(ctx.out(),
                        "x {} y {} {} {} l {} i_a {} e_a {} o_a {} i_l {} "
                        "e_l {} o_l {} n_n:{} fnc:{} fnr:{} fsr:{}",
                        p.getX(), p.getY_c(), p.getXw(), p.getYw(), p.getLevel(),
                        p.getInputAddr_c(), p.getEdgeAddr(), p.getOutputAddr(),
                        p.getInputLen(), p.getEdgeLen(), p.getOutputLen(),
                        p.getNumEdgesInWindow(), p.isTheFinalCol(),
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

class [[maybe_unused]] Slide_window_set {
public:
  Slide_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS,
                   std::vector<int> ywS, std::vector<int> nodeDimS,
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

  std::vector<int> node_dim_s;

  int total_level;

  std::vector<Slide_window> m_sliding_window_vec;
  // 3 dimension vector: level-col-row-element
  // m_sliding_window_multi_level[1][2][3],layer,1,col,2,row3
  std::vector<std::vector<std::vector<Slide_window>>>
      m_sliding_window_multi_level;

  std::vector<uint64_t> node_addrs; // for each level;
  // uint number_of_nodes_to_be_read = 0;
};

#endif // GCN_SIM_SLIDE_WINDOW_H_OLD
