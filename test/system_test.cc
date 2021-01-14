//
// Created by sjq on 1/13/21.
//

#include "System.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "vector"
TEST_CASE("system_test", "[big]") {
  spdlog::set_level(spdlog::level::debug);
  std::vector<int> node_sizes = {10, 20, 10};

  System m_system(1000, 1000, 1000, 1000, 16, 10, 10, "cora", node_sizes,
                  "DDR4-config.cfg");

  m_system.run();
  return;
}