#ifndef GRAPH_H
#define GRAPH_H

#include <fmt/core.h>
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
class Graph {
  using ull = uint64_t;
  // the csc format graph

public:
  // for simulation, we should get the dege index addr when to access the dege
  // index
  [[nodiscard]] unsigned get_num_nodes() const { return edge_index.size() - 1; }

  [[nodiscard]] ull get_edge_index_addr(unsigned index) const {
    return (ull) & (edge_index.at(index));
  }

  [[nodiscard]] ull get_edge_addr(unsigned index) const { return (ull) & (edges.at(index)); }

  [[nodiscard]] unsigned get_edge_size(unsigned index) const {
    return edge_index.at(index + 1) - edge_index.at(index);
  }

  [[nodiscard]] const std::vector<unsigned> &get_edge_index() const { return edge_index; }

  [[nodiscard]] const std::vector<unsigned> &get_edges() const { return edges; }

  [[nodiscard]] const std::vector<std::vector<double>> &get_nodes() const { return nodes; }

  explicit Graph(const std::string& name) { this->parse(name); }

  void parse(const std::string& graph_name);

  void print() const;

private:
  std::vector<unsigned> edge_index;
  std::vector<unsigned> edges;
  std::vector<std::vector<double>> nodes;
  int node_features{0};

public:
  [[nodiscard]] int getNodeFeatures() const;
};

#endif