#include "catch2/catch_test_macros.hpp"

#include <sliding_window_dense.h>
#include <fmt/format.h>
#include <graph.h>
#include <iostream>
#include <string>
/*
TEST_CASE("slid_test") {
    auto m_graph = std::make_shared<Graph>("test");
    Slide_window_set m_set(m_graph, {4, 2, 3}, {4, 2, 5}, {1, 2, 3, 1}, 4);
    const std::vector<dense_window> &w = m_set.get_windows();
    for (const Slide_window &i :w) {
        std::cout << fmt::format("{}", i) << std::endl;
    }

}
 */