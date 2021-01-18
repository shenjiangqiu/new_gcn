#include "Model.h"
#include "System.h"
#include "globals.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include "utils/Options.h"
#include <fmt/format.h>
#include <fstream>
#include <graph.h>
#include <iostream>

using namespace Minisat;

int main(int argc, char **argv) {
  std::map<std::string, int> node_feat_sizes = {{"cora", 1433},
                                                {"citeseer", 3703}};
  Minisat::parseOptions(argc, argv, false);
  // TODO: should be read gcn number
  // TODO: the agg_buffer might contain double information
  // the features dimension for each layer
  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;

  if (std::string(config::model) == std::string("gcn")) {
    m_model = std::shared_ptr<Model>(new Model("gcn", {128}, true));
  } else {
    throw std::runtime_error("no such model!");
  }
  if (m_model->isConcatenate()) {
    global_definitions.cycle = true;
  }

  if (node_feat_sizes.count(std::string(config::graph_name))) {
    node_feature_size = node_feat_sizes.at(std::string(config::graph_name));
  }

  if (config::debug) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::err);
  }

  std::shared_ptr<Graph> m_graph =
      std::make_shared<Graph>(std::string(config::graph_name));

  std::vector<int> node_sizes = {node_feature_size};
  node_sizes.insert(node_sizes.end(), m_model->getMLevels().begin(),
                    m_model->getMLevels().end());
  spdlog::info("print out model levels {}", fmt::join(node_sizes, ","));

  System m_system(config::inputSize, config::edgeSize, config::aggSize,
                  config::outputSize, config::aggCores, config::systolic_rows,
                  config::systolic_cols, m_graph, node_sizes,
                  (std::string)config::dram_name, m_model);
  m_system.run();

  return 0;
}