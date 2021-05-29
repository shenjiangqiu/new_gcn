#include "Model.h"
#include "System.h"
#include "globals.h"
#include "spdlog/spdlog.h"
#include "utils/Options.h"
#include <controller.h>
#include <debug_helper.h>
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

  if (m_model->isInitialResidual()) {
    global_definitions.initialResidual = true;
  }

  if (config::debug) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  GCN_INFO("Enable feature sparsity: {}", config::enable_feature_sparsity);

  std::shared_ptr<Graph> m_graph =
      std::make_shared<Graph>(std::string(config::graph_name));

  // check the size
  if (global_definitions.concate) {
    if (config::ignore_self >= m_graph->getNodeFeatures() or
        config::ignore_neighbor >= m_graph->getNodeFeatures()) {
      GCN_ERROR(
          "the ignored feature should be less than the node feature size, node "
          "feature size:{},ignore_neighbor:{},ignore_self:{}",
          m_graph->getNodeFeatures(), config::ignore_neighbor,
          config::ignore_self);
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("error in check size");
    }
  } else {
    if (config::ignore_self != 0) {
      GCN_ERROR_S(
          "when this is not concatenate, the ignore self should be zero!");
      spdlog::flush_on(spdlog::level::err);
      throw std::runtime_error("error in check size");
    }
    if (config::ignore_self >= m_graph->getNodeFeatures()) {
      GCN_ERROR(
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
  GCN_INFO("print out model levels {}", fmt::join(node_sizes, ","));

  GCN_INFO("memory simulator: {}", config::mem_sim);
  if (config::enable_fast_sched) {
    auto i_bf = std::make_shared<fast_sched::InputBuffer>();
    auto m_agg =
        std::make_shared<fast_sched::Aggregator_fast>(config::aggCores);
    auto m_mem = std::make_shared<memory_interface>("HBM-config.cfg", "", 64);
    std::vector<unsigned> m_node_sizes;
    std::vector<unsigned> m_input_num;
    std::vector<unsigned> m_output_num;
    for (auto i = 0; i < node_sizes.size() - 1; i++) {
      m_node_sizes.push_back(node_sizes[i]);
      m_input_num.push_back((config::inputSize / 2) / node_sizes[i]);
      m_output_num.push_back((config::aggSize / 2) / node_sizes[i]);
    }

    auto m_controller = fast_sched::controller(
        *m_graph, i_bf, m_node_sizes, m_input_num, m_output_num, m_agg, m_mem);
    uint64_t cycle = 0;
    uint mem_rount = 0;
    while (!m_controller.isAllFinished()) {
      
      m_controller.cycle();
      i_bf->cycle();
      m_agg->cycle();
      if (mem_rount == 0) {
        m_mem->cycle();
      }
      mem_rount++;
      mem_rount%=2;
      
      cycle++;
    }
    fmt::print("cycle: {}", cycle);

  } else {
    System m_system(config::inputSize, config::edgeSize, config::aggSize,
                    config::outputSize, config::aggCores, config::systolic_rows,
                    config::systolic_cols, m_graph, node_sizes,
                    (std::string)config::dram_name, m_model);
    m_system.run();
  }

  return 0;
}