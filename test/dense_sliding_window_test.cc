//
// Created by Jiangqiu shen on 5/5/21.
//
#include "catch2/catch.hpp"
#include "fmt/format.h"
#include "sliding_window_dense.h"
#include "generate_graph.h"
TEST_CASE("dense_sliding_window_test") {
  REQUIRE(1 == 1);

  auto m_graph = std::make_shared<Graph>("test");
  dense_window_set m_set(m_graph, {4, 2, 3}, {4, 2, 5}, {1, 2, 3, 1}, 4,true);
  for (auto &i : m_set) {
    std::cout << fmt::format("{}", *std::static_pointer_cast<dense_window>(i)) << std::endl;
  }
}