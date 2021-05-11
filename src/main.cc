#include "Model.h"
#include "System.h"
#include "globals.h"
#include "spdlog/spdlog.h"
#include "utils/Options.h"
#include <fmt/format.h>
#include <graph.h>

using namespace Minisat;

int main(int argc, char **argv) {

  Minisat::parseOptions(argc, argv, false);

  // TODO: should be read gcn number
  // TODO: the agg_buffer might contain double information
  // the features dimension for each layer
  std::shared_ptr<Model> m_model;
  int node_feature_size = 0;
  std::string model_name = std::string(config::model);
  if (!global_definitions.m_models.count(model_name)) {
    throw std::runtime_error("no such model");
  }
  m_model = std::make_shared<Model>(global_definitions.m_models[model_name]);

  if (m_model->isConcatenate()) {
    global_definitions.concate = true;
  }

  if (m_model->isInitialResidual() ) {
    global_definitions.initialResidual = true;
  }

  if (config::debug) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }
  
  spdlog::info("Enable feature sparsity: {}", config::enable_feature_sparsity);


  std::shared_ptr<Graph> m_graph =
      std::make_shared<Graph>(std::string(config::graph_name));

  // check the size
  if (global_definitions.concate) {
    if (config::ignore_self >= m_graph->getNodeFeatures() or
        config::ignore_neighbor >= m_graph->getNodeFeatures()) {
      spdlog::error(
          "the ignored feature should be less than the node feature size, node "
          "feature size:{},ignore_neighbor:{},ignore_self:{}",
          m_graph->getNodeFeatures(), config::ignore_neighbor,
          config::ignore_self);
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("error in check size");
    }
  } else {
    if (config::ignore_self != 0) {
      spdlog::error(
          "when this is not concatenate, the ignore self should be zero!");
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("error in check size");
    }
    if (config::ignore_self >= m_graph->getNodeFeatures()) {
      spdlog::error(
          "the ignored feature should be less than the node feature size, node "
          "feature size:{},ignore_neighbor:{}",
          m_graph->getNodeFeatures(), config::ignore_neighbor);
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("error in check size");
    }
  }

  node_feature_size = m_graph->getNodeFeatures();
  std::vector<int> node_sizes = {node_feature_size};
  node_sizes.insert(node_sizes.end(), m_model->getMLevels().begin(),
                    m_model->getMLevels().end());
  spdlog::info("print out model levels {}", fmt::join(node_sizes, ","));

  spdlog::info("memory simulator: {}", config::mem_sim);

  System m_system(config::inputSize, config::edgeSize, config::aggSize,
                  config::outputSize, config::aggCores, config::systolic_rows,
                  config::systolic_cols, m_graph, node_sizes,
                  (std::string)config::dram_name, m_model);
  m_system.run();

  return 0;
}