#include <fstream>
#include <graph.h>
#include <sstream>

#include "utils/Options.h"
#include <iostream>
using namespace Minisat;
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
    getline(graph_in, i_line);
    if (graph_in.eof())
      break;
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
}
int Graph::getNodeFeatures() const { return node_features; }

// those are global definitions, do not use them in constructors!!!
Minisat::IntOption inputSize("system", "input", "the input buffer size(Byte)");
Minisat::IntOption outputSize("system", "output",
                              "the output buffer size(Byte)");
Minisat::IntOption edgeSize("system", "edge", "the edge buffer size(Byte)");
Minisat::IntOption aggSize("system", "agg", "the agg buffer size(Byte)");

Minisat::IntOption aggCores("system", "aggCores", "the agg buffer size(Byte)");
Minisat::IntOption systolic_rows("system", "systolic-rows",
                                 "the agg buffer size(Byte)");
Minisat::IntOption systolic_cols("system", "systolic-cols",
                                 "the agg buffer size(Byte)");
Minisat::StringOption graph_name("system", "graph-name", "the name of graph",
                                 "test");
Minisat::StringOption dram_name("sym", "dram-name", "the name of dram",
                                "DDR4-config.cfg");
Minisat::BoolOption debug("system", "debug", "if enable debug", false);

Minisat::StringOption model("system", "model", "the model definition file",
                            "gcn");
Minisat::BoolOption double_input(
    "system", "double-input",
    "will the result of aggregation need to concat the origin vector", false);
