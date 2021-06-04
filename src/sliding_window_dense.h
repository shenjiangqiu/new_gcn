//
// Created by sjq on 3/26/21.
//

#ifndef GCN_SIM_SLIDING_WINDOW_DENSE_H
#define GCN_SIM_SLIDING_WINDOW_DENSE_H

#include "fmt/core.h"
#include "fmt/format.h"
#include "sliding_window_interface.h"
#include <algorithm>
#include <graph.h>
#include <iostream>
#include <memory>
class dense_window : public sliding_window_interface {
public:
  unsigned int getYw() const override;
  void set_addr(uint64_t inputAddr, unsigned int inputLen, uint64_t edgeAddr,
                unsigned int edgeLen, uint64_t outputAddr,
                unsigned int outputLen) override;

  void set_location(unsigned int x, unsigned int _xw, unsigned int _y,
                    unsigned int yw, unsigned int _level) override;
  [[nodiscard]] unsigned int getY_c() const override;
  [[nodiscard]] uint64_t getInputAddr_c() const override;

public:
  bool operator==(const dense_window &rhs) const;

  bool operator!=(const dense_window &rhs) const;

  dense_window() = default;
  virtual ~dense_window() = default;
  // setting the window size and location,x:begin of x, _xw:x width
  // _y:all rows
  void set_location(unsigned x, unsigned _xw, std::vector<unsigned> _y,
                    unsigned _level) override;

  void set_addr(std::vector<uint64_t> inputAddr, unsigned inputLen,
                uint64_t edgeAddr, unsigned edgeLen, uint64_t outputAddr,
                unsigned outputLen) override;

  void set_size(unsigned currentEdges, unsigned currentnodeDim) override;

  void set_prop(bool the_final_col, bool theFinalRow, bool theFirstRow,
                bool the_final_layer) override;

  [[nodiscard]] unsigned getX() const override;

  [[nodiscard]] std::vector<unsigned int> getY() const override;

  [[nodiscard]] unsigned getXw() const override;

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
  [[nodiscard]] bool isTheFinalLayer() const override {
    return the_final_layer;
  }
  void setTheFinalRow(bool theFinalRow) override;

  [[nodiscard]] unsigned getNumEdgesInWindow() const override;

private:
  unsigned x{};            // x: starting vertex ID in the aggregation buffer
  std::vector<unsigned> y; // y: starting vertex ID in the input buffer
  unsigned y_c{};          // yc: the continues type of y
  unsigned xw{};           // cnt of vertices in the the aggregation buffer
  unsigned level{};
  std::vector<uint64_t> input_addr;
  uint64_t input_addr_c{}; // input_addr_c: the continues type of addr

  uint64_t edge_addr{}, output_addr{};
  unsigned input_len{}, edge_len{}, output_len{};
  unsigned num_edges_in_window{}; //#edges in the window
  unsigned current_node_dim{};   // A feature dim.
  bool the_final_col{};
  bool the_final_layer{};

  bool the_final_row{};
  bool the_first_row{};
};

template <> struct fmt::formatter<dense_window> {
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
  auto format(const dense_window &p, FormatContext &ctx) {
    // auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) //
    // c++11 ctx.out() is an output iterator to write to.
    auto out =
        simple ? format_to(ctx.out(), "{} {} {} {} fnc:{} fnr:{} fsr:{} fnl:{]",
                           p.getX(), fmt::join(p.getY(), ","), p.getXw(),
                           p.getLevel(), p.isTheFinalCol(), p.isTheFinalRow(),
                           p.isTheFirstRow(), p.isTheFinalLayer())
               : format_to(ctx.out(),
                           "x {} y {}  {} l {} i_a {} e_a {} o_a {} i_l {} "
                           "e_l {} o_l {} n_n:{} fnc:{} fnr:{} fsr:{} fnl:{}",
                           p.getX(), fmt::join(p.getY(), ","), p.getXw(),
                           p.getLevel(), fmt::join(p.getInputAddr(), ","),
                           p.getEdgeAddr(), p.getOutputAddr(), p.getInputLen(),
                           p.getEdgeLen(), p.getOutputLen(),
                           p.getNumEdgesInWindow(), p.isTheFinalCol(),
                           p.isTheFinalRow(), p.isTheFirstRow(),
                           p.isTheFinalLayer());
    return out;
  }
};
using dense_window_iter = std::vector<dense_window>::iterator;
class dense_window_set {
public:
  dense_window_set(std::shared_ptr<Graph> mGraph, std::vector<int> xwS,
                   std::vector<int> ywS, std::vector<int> nodeDimS,
                   int totalLevel, bool is_dense);

  const std::vector<std::shared_ptr<sliding_window_interface>> &get_windows() {
    return m_sliding_window_vec;
  }

  std::vector<std::shared_ptr<sliding_window_interface>>::iterator begin();

  std::vector<std::shared_ptr<sliding_window_interface>>::iterator end();

private:
  std::shared_ptr<Graph> m_graph;
  std::vector<int> xw_s;
  std::vector<int> yw_s;

  std::vector<int> node_dim_s;

  int total_level;

  std::vector<std::shared_ptr<sliding_window_interface>> m_sliding_window_vec;

  std::vector<uint64_t> node_addrs; // for each level;
  // unsigned number_of_nodes_to_be_read = 0;
};

#endif // GCN_SIM_SLIDING_WINDOW_DENSE_H
