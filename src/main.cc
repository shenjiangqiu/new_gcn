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
  // this value must be init after parse option, because it depends on graph
  // name
  auto options = Minisat::Option::getOptionList();

  for (auto &&i : options) {
    GCN_INFO("name:{} value:{}", i->get_name(), i->get_value());
  }
  global_definitions.edge_agg_logger = spdlog::basic_logger_st(
      "trace_edge_agg.txt",
      fmt::format("logs/{}/edge_agg_{}_{}.txt", config::graph_name,
                  config::hash_table_size, config::short_large_divider),
      true);
  global_definitions.default_logger = spdlog::basic_logger_st(
      "default_logger.txt",
      fmt::format("logs/{}/default_logger_{}_{}.txt", config::graph_name,
                  config::hash_table_size, config::short_large_divider),
      true);

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
    global_definitions.default_logger->set_level(spdlog::level::debug);
    spdlog::set_level(spdlog::level::debug);
  } else {
    global_definitions.default_logger->set_level(spdlog::level::info);
    spdlog::set_level(spdlog::level::info);
  }

  GCN_INFO("Enable feature sparsity: {}", config::enable_feature_sparsity);

  std::shared_ptr<Graph> m_graph =
      std::make_shared<Graph>(std::string(config::graph_name));
  if (config::enable_global_sorted_graph) {
    m_graph->sort_translate();
  }
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
  std::vector<int> node_dim_per_layer = {node_feature_size};
  node_dim_per_layer.insert(node_dim_per_layer.end(),
                            m_model->getMLevels().begin(),
                            m_model->getMLevels().end());
  GCN_INFO("print out model levels {}", fmt::join(node_dim_per_layer, ","));

  GCN_INFO("memory simulator: {}", config::mem_sim);
  // load names
  std::vector<std::string> in_names;
  std::vector<std::string> mask_names;
  if (config ::enable_sparse) {
    std::string temp;
    std::string origin_in_names = std::string(config::in_names);
    std::stringstream ss(origin_in_names);
    while (std::getline(ss, temp, ',')) {
      in_names.push_back(temp);
    }

    std::string origin_mask_names = std::string(config::mask_names);
    std::stringstream ss3(origin_mask_names);
    while (std::getline(ss3, temp, ',')) {
      mask_names.push_back(temp);
    }
  }
  if (config::enable_fast_sched) {
    auto i_bf = std::make_shared<fast_sched::InputBuffer>();
    auto m_agg =
        std::make_shared<fast_sched::Aggregator_fast>(config::aggCores);
    auto m_mem = std::make_shared<memory_interface>("HBM-config.cfg", "", 64);
    std::vector<unsigned> m_node_feature_dim;
    std::vector<unsigned> m_input_num;
    // fix bug here, the input num should be changed for sparse input

    // fix bug here, nodeDim is the number of features per node, not the real
    // node size,//maybe rename it? do it!
    for (auto i = 0u; i < node_dim_per_layer.size() - 1; i++) {
      m_node_feature_dim.push_back(node_dim_per_layer[i]);
      m_input_num.push_back((config::inputSize / 2) /
                            (node_dim_per_layer[i] * 4));
    }
    GCN_INFO("the input num: {}", fmt::join(m_input_num, ","));
    GCN_INFO("the nodeDim num: {}", fmt::join(node_dim_per_layer, ","));
    GCN_INFO("the buffersize num: {}", config::inputSize);

    // STEP 1: create the controller
    auto m_controller = fast_sched::controller(
        *m_graph, i_bf, m_node_feature_dim, m_input_num, m_agg, m_mem,
        config::short_large_divider, config::short_queue_size,
        config::large_queue_size, config::task_queue_size, config::aggSize,
        config::enable_outer_list, std::string(config::outer_name), in_names,
        mask_names);

    global_definitions.cycle = 0;
    uint mem_round = 0;
    while (!m_controller.isAllFinished()) {

      m_controller.cycle();
      i_bf->cycle();
      m_agg->cycle();
      if (mem_round == 0) {
        m_mem->cycle();
      }
      mem_round++;
      mem_round %= 2;

      global_definitions.cycle++;
    }
    m_controller.print();
    fmt::print("cycle: {}\n", global_definitions.cycle);
    fmt::print("total_agg: {}\n", m_agg->get_total_operations());
    fmt::print("total_rounds_in_agg: {}\n", m_agg->get_total_rounds());
    fmt::print("do_agg: {}\n", global_definitions.do_aggregate);
    fmt::print("agg_ops: {}\n", global_definitions.total_aggregate_op);
    fmt::print("global_definitions.total_input_windows: {}\n",
               global_definitions.total_input_windows);
    fmt::print("global_definitions.total_edges: {}\n",
               global_definitions.total_edges);

    fmt::print("total_read_traffic: {}\n",
               global_definitions.total_read_traffic);
    fmt::print("total_write_traffic: {}\n",
               global_definitions.total_write_traffic);
    fmt::print("total_read_traffic_input: {}\n",
               global_definitions.total_read_traffic_input);

    fmt::print("total_read_traffic_edge: {}\n",
               global_definitions.total_read_traffic_edge);
    fmt::print("output the histo for query_and_delete\n");
    for (auto i : global_definitions.number_to_count_map_for_query) {
      fmt::print("{} : {}\n", i.first, i.second);
    }
    fmt::print("cycle_insert_hash: {}\n",
               global_definitions.total_cycle_insert_hash_table);
    fmt::print("cycle_query_hash: {}\n",
               global_definitions.total_cycle_query_hash_table);
    fmt::print("hop_histo:\n");
    for (auto i : global_definitions.number_hops_histogram) {
      fmt::print("{} : {}\n", i.first, i.second);
    }
    fmt::print("controller_layer_0_window:{}\n",
               global_definitions.controlloer_layer_0_windows);
    fmt::print("controller_layer_0_edges:{}\n",
               global_definitions.controller_layer_0_edges);

    fmt::print("sliding_window_input_buffer_nodes: {}\n",
               global_definitions.sliding_window_input_buffer_nodes);
    fmt::print("sliding_window_input_nodes: {}\n",
               global_definitions.sliding_window_input_nodes);
    fmt::print("sliding_window_effect_input_nodes: {}\n",
               global_definitions.sliding_window_effect_input_nodes);
    fmt::print("total_edges_in_window: {}\n",
               global_definitions.total_edges_in_window);
    fmt::print("total_window_size: {}\n", global_definitions.total_window_size);
    fmt::print("input_traffic: {}\n", global_definitions.input_traffic);

    // sparse related cycles
    fmt::print("sparse add: {}\n", global_definitions.sparse_agg_cycles);
    fmt::print("sparse_mult_cycles: {}\n",
               global_definitions.sparse_mult_cycles);
    fmt::print("sparse_mask_cycles: {}\n",
               global_definitions.sparse_mask_cycles);

  } else {
    std::shared_ptr<sparseVector> m_vec;
    m_vec = config::enable_sparse
                ? std::make_shared<sparseVector>(in_names, mask_names)
                : nullptr;
    System m_system(config::inputSize, config::edgeSize, config::aggSize,
                    config::outputSize, config::aggCores, config::systolic_rows,
                    config::systolic_cols, m_graph, node_dim_per_layer,
                    (std::string)config::dram_name, m_model, m_vec,
                    config::enable_sparse);
    m_system.run();

    fmt::print("sliding_window_input_buffer_nodes: {}\n",
               global_definitions.sliding_window_input_buffer_nodes);
    fmt::print("sliding_window_input_nodes: {}\n",
               global_definitions.sliding_window_input_nodes);
    fmt::print("sliding_window_effect_input_nodes: {}\n",
               global_definitions.sliding_window_effect_input_nodes);
    fmt::print("total_edges_in_window: {}\n",
               global_definitions.total_edges_in_window);
    fmt::print("total_window_size: {}\n", global_definitions.total_window_size);
    fmt::print("input_traffic: {}\n", global_definitions.input_traffic);
  }

  return 0;
}