//
// Created by Jiangqiu shen on 5/17/21.
//

#include "Slide_window.h"
#include "System.h"
#include "catch2/catch.hpp"
#include "fmt/format.h"
#include "globals.h"
#include "graph.h"
#include "spdlog/spdlog.h"
TEST_CASE("mix_test") {
  // first test dense
  char **argv = new char *[10];
  argv[0] = new char[10];
  argv[1] = new char[10];
  spdlog::set_level(spdlog::level::info);

  strcpy(argv[0], "test");
  strcpy(argv[1], "-enable-dense-window");
  int argc = 2;
  Minisat::parseOptions(argc, argv, false);
  fmt::print("{}\n", (bool)config::enable_dense_window);
  auto m_graph = std::make_shared<Graph>("test");

  dense_window_set m_set(m_graph, {3, 3, 3}, {3, 3, 3}, {10, 10, 10, 10}, 4,
                         true);

  for (const auto &window : m_set) {
    fmt::print("{}\n", *std::static_pointer_cast<dense_window>(window));
  }
  // xws will be:3,3,7 yws will be 3,3,7
  std::vector<int> node_sizes = {10, 10, 10};
  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;

  m_model = std::make_shared<Model>(global_definitions.m_models["gcn"]);

  System m_system(300, 100, 300, 100, 1, 4, 4, m_graph, node_sizes,
                  (std::string)config::dram_name, m_model);

  m_system.run();
  delete[] argv[0];
  delete[] argv[1];
  delete[] argv;
}

TEST_CASE("cont_test") {

  char **argv = new char *[10];
  argv[0] = new char[10];
  argv[1] = new char[10];
  spdlog::set_level(spdlog::level::info);

  strcpy(argv[0], "test");
  strcpy(argv[1], "-no-enable-dense-window");
  int argc = 2;
  Minisat::parseOptions(argc, argv, false);
  fmt::print("{}\n", (bool)config::enable_dense_window);
  auto m_graph = std::make_shared<Graph>("test");

  dense_window_set m_set(m_graph, {3, 3, 3}, {3, 3, 3}, {10, 10, 10, 10}, 4,
                         false);

  for (const auto &window : m_set) {
    fmt::print("{}\n", *std::static_pointer_cast<Slide_window>(window));
  }
  // xws will be:3,3,7 yws will be 3,3,7
  std::vector<int> node_sizes = {10, 10, 10};
  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;

  m_model = std::make_shared<Model>(global_definitions.m_models["gcn"]);

  System m_system(300, 100, 300, 100, 1, 4, 4, m_graph, node_sizes,
                  (std::string)config::dram_name, m_model);

  m_system.run();
  delete[] argv[0];
  delete[] argv[1];
  delete[] argv;
}