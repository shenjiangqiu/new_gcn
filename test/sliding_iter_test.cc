//
// Created by sjq on 1/10/21.
//
#include "Slide_window.h"
#include "catch.hpp"

TEST_CASE("sliding_iter") {
  auto m_graph = std::make_shared<Graph>("test");
  Slide_window_set m_set(m_graph, {4, 2, 3}, {4, 2, 5}, {1, 2, 3, 1}, 4);
  const std::vector<Slide_window> &w = m_set.get_windows();

  auto i = w.begin();
  auto j = m_set.begin();
  for (; i != w.end() and j != m_set.end(); i++, j++) {
    REQUIRE(*i == *j);
    fmt::print("{:s}\n", *i);
  }
  j = m_set.begin();
  fmt::print("next: {:s}\n", *j);

  auto next_col = j.get_next_col();
  fmt::print("next: {:s}\n", *next_col);

  while (next_col.have_next_col()) {
    next_col = next_col.get_next_col();
    fmt::print("next: {:s}\n", *next_col);
  }
}
