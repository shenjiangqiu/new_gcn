#include <fstream>
#include <graph.h>
#include <sstream>

#include "utils/Options.h"
#include <debug_helper.h>
#include <iostream>
using namespace Minisat;
// find bug here, need to trail the tail line
void Graph::parse(const std::string &graph_name) {
  fmt::print("parse graph:{}\n", graph_name);
  std::string full_graph_name = graph_name + ".graph";
  std::ifstream graph_in(full_graph_name);
  if (!graph_in.is_open()) {
    throw std::runtime_error("cannot open the file");
  }
  int node = 0;
  edge_index.push_back(0);
  std::string line;
  getline(graph_in, line);
  if (graph_in.eof())
    throw std::runtime_error("invalid graph format");
  std::stringstream ss(line);
  char f;
  ss >> f;
  if (f != 'f') {
    throw std::runtime_error("first line should be f {number}");
  }
  int feature_size = 0;
  ss >> feature_size;
  node_features = feature_size;
  assert(feature_size > 0);
  while (true) {
    std::string i_line;
    if (graph_in.eof())
      break;
    getline(graph_in, i_line);
    // fix bug here, now use exciplite end sign
    if (i_line.starts_with("end") or i_line.starts_with("END")) {
      break;
    }
    edges.push_back(node);
    node++;
    edge_index.push_back(edge_index.back() + 1);
    std::stringstream i_ss(i_line);
    while (true) {
      int neighbor;
      i_ss >> neighbor;
      if (i_ss.fail())
        break;
      edges.push_back(neighbor);
      edge_index.back() += 1;
    }
  }
}
void Graph::print() const {
  for (auto index : edge_index) {
    std::cout << index << " ";
  }
  std::cout << std::endl;
  for (auto e : edges) {
    std::cout << e << " ";
  }
  std::cout << std::endl;

  if (sorted) {
    std::cout << "sorted\n";

    for (auto index : edge_index_sorted) {
      std::cout << index << " ";
    }

    std::cout << std::endl;

    for (auto e : edges_sorted) {
      std::cout << e << " ";
    }

    std::cout << std::endl;

  } else {
    std::cout << "not_sorted" << std::endl;
  }
}

int Graph::getNodeFeatures() const { return node_features; }

// // those are global definitions, do not use them in constructors!!!
// Minisat::IntOption inputSize("system", "input", "the input buffer
// size(Byte)"); Minisat::IntOption outputSize("system", "output",
//                               "the output buffer size(Byte)");
// Minisat::IntOption edgeSize("system", "edge", "the edge buffer size(Byte)");
// Minisat::IntOption aggSize("system", "agg", "the agg buffer size(Byte)");

// Minisat::IntOption aggCores("system", "aggCores", "the agg buffer
// size(Byte)"); Minisat::IntOption systolic_rows("system", "systolic-rows",
//                                  "the agg buffer size(Byte)");
// Minisat::IntOption systolic_cols("system", "systolic-cols",
//                                  "the agg buffer size(Byte)");
// Minisat::StringOption graph_name("system", "graph-name", "the name of graph",
//                                  "test");
// Minisat::StringOption dram_name("sym", "dram-name", "the name of dram",
//                                 "DDR4-config.cfg");
// Minisat::BoolOption debug("system", "debug", "if enable debug", false);

// Minisat::StringOption model("system", "model", "the model definition file",
//                             "gcn");
// Minisat::BoolOption double_input(
//     "system", "double-input",
//     "will the result of aggregation need to concat the origin vector",
//     false);
void Graph::sort_translate() {
  GCN_INFO_S("start to sort the graph");
  if (sorted) {
    GCN_INFO_S("already sorted");

    return;
  }
  std::vector<std::pair<unsigned, unsigned>> number;
  for (auto i = 1u; i < edge_index.size(); i++) {
    // number[0]={0,ei[1]-ei[0]};
    number.push_back({i - 1, edge_index[i] - edge_index[i - 1]});
  }
  GCN_INFO("finished build the number count:size:{}", number.size());

  std::sort(number.begin(), number.end(), [&](auto &&pair1, auto &&pair2) {
    return pair1.second < pair2.second;
  });
  GCN_INFO_S("finished sort the edges");

  std::vector<unsigned> old_to_new_mapping(number.size());
  for (auto i = 0u; i < number.size(); i++) {
    try {
      old_to_new_mapping.at(number.at(i).first) = i;
    } catch (std::exception &e) {
      GCN_ERROR("ERROR, fail to build the old to new mapping, when "
                "processing the new number:{}",
                i);
      if (number.size() > i) {
        GCN_ERROR("the old number is: {}", number.at(i).first);
        GCN_ERROR("but the old_to_new_mapping is: {}",
                  old_to_new_mapping.size());
      } else {

        throw;
      }

      throw;
    }
  }
  GCN_INFO("finished build old to new mappting, mapping size:{}",
           old_to_new_mapping.size());

  edge_index_sorted.push_back(0);
  for (auto i = 1u; i < edge_index.size(); i++) {
    // fill the new edge index
    try {
      edge_index_sorted.push_back(edge_index_sorted[i - 1] +
                                  number[i - 1].second);
    } catch (std::exception &e) {
      GCN_ERROR("edge index sorted pushback error??:{}",
                edge_index_sorted.size());

      throw;
    }
  }
  for (auto i = 0u; i < number.size(); i++) {
    // noticed here, need to remap the edge node id according to the new array
    auto node_id = number[i].first;

    auto original_idx_start = edge_index[node_id];
    auto original_idx_end = edge_index[node_id + 1];
    assert((original_idx_end - original_idx_start) == number[i].second);
    for (auto j = original_idx_start; j < original_idx_end; j++) {
      try {
        edges_sorted.push_back(old_to_new_mapping.at(edges.at(j)));
      } catch (std::exception &e) {
        fmt::print("{}\n", e.what());
        GCN_ERROR("fail to push the translated edges, old edge index:{}", j);
        if (edges.size() > j) {
          auto edge_number = edges.at(j);
          GCN_ERROR("the edge number at {} is {}", j, edges.at(j));
          if (old_to_new_mapping.size() > edge_number) {
            GCN_ERROR("new mapping numboer for this edge is {}",
                      old_to_new_mapping.at(edges.at(j)));

          } else {
            GCN_ERROR("fail to find the mapping, the maping size is:{},but the "
                      "odl edge is:{}",
                      old_to_new_mapping.size(), edges.at(j));
          }
        } else {
          GCN_ERROR("this edge not exist:{}, the edge size:{}", j,
                    edges.size());
        }

        global_definitions.default_logger->flush();
        global_definitions.edge_agg_logger->flush();
        throw;
      }
    }
  }
  sorted = true;
}