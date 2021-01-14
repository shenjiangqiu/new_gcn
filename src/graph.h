#ifndef GRAPH_H
#define GRAPH_H

#include "fmt/format.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
class Graph {
  using ull = unsigned long long;
  // the csc format graph
private:
  std::vector<unsigned> edge_index;
  std::vector<unsigned> edges;
  std::vector<std::vector<double>> nodes;

public:
  // for simulation, we should get the dege index addr when to access the dege
  // index
  unsigned get_num_nodes() const { return edge_index.size() - 1; }

  ull get_edge_index_addr(unsigned index) const {
    return (ull) & (edge_index.at(index));
  }

  ull get_edge_addr(unsigned index) const { return (ull) & (edges.at(index)); }

  unsigned get_edge_size(unsigned index) const {
    return edge_index.at(index + 1) - edge_index.at(index);
  }

  const std::vector<unsigned> &get_edge_index() const { return edge_index; }

  const std::vector<unsigned> &get_edges() const { return edges; }

  const std::vector<std::vector<double>> &get_nodes() const { return nodes; }

  Graph(const std::string name) { this->parse(name); }

  void parse(const std::string& graph_name);

  void print() const {
    for (auto index : edge_index) {
      std::cout << index << " ";
    }
    std::cout << std::endl;
    for (auto e : edges) {
      std::cout << e << " ";
    }
    std::cout << std::endl;
  }
};

#endif