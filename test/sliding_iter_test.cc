//
// Created by sjq on 1/10/21.
//
#include "sliding_window_dense.h"
#include "catch2/catch.hpp"

/*
TEST_CASE("sliding_iter") {
  auto m_graph = std::make_shared<Graph>("cora");
  Slide_window_set m_set(m_graph, {64, 128, 64}, {64, 128, 64}, {1, 2, 3, 1},
                         4);
  const std::vector<dense_window> &w = m_set.get_windows();

  auto i = w.begin();
  auto j = m_set.begin();
  for (; i != w.end() and j != m_set.end(); i++, j++) {
    REQUIRE(*i == *j);
    fmt::print("{}\n", *i);
  }
  j = m_set.begin();
  fmt::print("next: {}\n", *j);

  auto next_col = j.get_next_col();
  fmt::print("next: {}\n", *next_col);

  while (next_col.have_next_col()) {
    next_col = next_col.get_next_col();
    fmt::print("next: {}\n", *next_col);
  }
  FILE *f = fopen("out.vec", "wb");

  for (auto &w : m_set) {
    fmt::print(f, "{} {} {} {} {} {} {}\n", w.getX(), w.getY(), w.getXw(),
               w.getYw(), w.getLevel(), w.isTheFirstRow(), w.isTheFinalRow());
  }
  fclose(f);
}
*/