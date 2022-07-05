#include <catch2/catch_test_macros.hpp>

#include <generate_graph.h>
#include <graph.h>
TEST_CASE("sorted_graph_test") {
  generate_small_graph();
  auto m_graph = Graph("test");
  m_graph.sort_translate();
  m_graph.print();
}