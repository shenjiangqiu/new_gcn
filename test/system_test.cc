//
// Created by sjq on 1/13/21.
//

#include "System.h"
#include "catch2/catch.hpp"

#include "globals.h"
#include "spdlog/spdlog.h"
#include "vector"

TEST_CASE("system_graph_test") {

  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;

  m_model = std::make_shared<Model>(global_definitions.m_models["gcn"]);

  if (m_model->isConcatenate()) {
    global_definitions.concate = true;
  }

  spdlog::set_level(spdlog::level::debug);

  std::shared_ptr<Graph> m_graph = std::make_shared<Graph>(std::string("test"));

  node_feature_size = m_graph->getNodeFeatures();
  std::vector<int> node_sizes = {node_feature_size};
  node_sizes.insert(node_sizes.end(), m_model->getMLevels().begin(),
                    m_model->getMLevels().end());
  spdlog::info("print out model levels {}", fmt::join(node_sizes, ","));

  // System m_system(300, 100, 300, 100, 1, 4, 4, m_graph, node_sizes,
  //                 (std::string)config::dram_name, m_model);
  // auto windows = m_system.get_sliding_window();

}
TEST_CASE("system_test", "[big]") {

  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;

  m_model = std::make_shared<Model>(global_definitions.m_models["gcn"]);

  if (m_model->isConcatenate()) {
    global_definitions.concate = true;
  }

  spdlog::set_level(spdlog::level::debug);

  std::shared_ptr<Graph> m_graph = std::make_shared<Graph>(std::string("test"));

  node_feature_size = m_graph->getNodeFeatures();
  std::vector<int> node_sizes = {node_feature_size};
  node_sizes.insert(node_sizes.end(), m_model->getMLevels().begin(),
                    m_model->getMLevels().end());
  spdlog::info("print out model levels {}", fmt::join(node_sizes, ","));

  // System m_system(300, 100, 300, 100, 1, 4, 4, m_graph, node_sizes,
  //                 (std::string)config::dram_name, m_model);
  // m_system.run();
}