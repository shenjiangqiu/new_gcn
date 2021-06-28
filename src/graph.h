#ifndef GRAPH_H
#define GRAPH_H

#include <array>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
class Graph {
  // the csc format graph

public:
  // for simulation, we should get the edge index addr when to access the dege
  // index
  [[nodiscard]] unsigned get_num_nodes() const { return edge_index.size() - 1; }

  [[nodiscard]] uint64_t get_edge_index_addr(unsigned index) const {
    return (uint64_t) & (edge_index.at(index));
  }

  [[nodiscard]] uint64_t get_edge_addr(unsigned index) const {
    return (uint64_t) & (edges.at(index));
  }

  [[nodiscard]] unsigned get_edge_size(unsigned index) const {
    return edge_index.at(index + 1) - edge_index.at(index);
  }

  [[nodiscard]] const std::vector<unsigned> &get_edge_index() const {
    return edge_index;
  }

  [[nodiscard]] const std::vector<unsigned> &get_edges() const { return edges; }

  [[nodiscard]] const std::vector<std::vector<double>> &get_nodes() const {
    return nodes;
  }

  explicit Graph(const std::string &name) { this->parse(name); }

  void parse(const std::string &graph_name);

  void print() const;

  void sort_translate() {
    if (sorted) {
      return;
    }
    std::vector<std::pair<unsigned, unsigned>> number;
    for (auto i = 1u; i < edge_index.size(); i++) {
      // number[0]={0,ei[1]-ei[0]};
      number.push_back({i - 1, edge_index[i] - edge_index[i - 1]});
      std::cout << "pusing back:" << i - 1 << ","
                << edge_index[i] - edge_index[i - 1] << std::endl;
    }

    std::sort(number.begin(), number.end(), [&](auto &&pair1, auto &&pair2) {
      return pair1.second < pair2.second;
    });
    for (auto i : number) {
      fmt::print("new number: {},{}\n", i.first, i.second);
    }

    std::vector<unsigned> old_to_new_mapping(number.size());
    for (auto i = 0u; i < number.size(); i++) {
      fmt::print("pushing back:old: {},new:{}\n", number.at(i).first, i);
      old_to_new_mapping.at(number.at(i).first) = i;
    }

    edge_index_sorted.push_back(0);
    for (auto i = 1u; i < edge_index.size(); i++) {
      // fill the new edge index
      edge_index_sorted.push_back(edge_index_sorted[i - 1] +
                                  number[i - 1].second);
    }
    for (auto i = 0u; i < number.size(); i++) {
      // noticed here, need to remap the edge node id according to the new array
      auto node_id = number[i].first;

      auto original_idx_start = edge_index[node_id];
      auto original_idx_end = edge_index[node_id + 1];
      assert((original_idx_end - original_idx_start) == number[i].second);
      for (auto j = original_idx_start; j < original_idx_end; j++) {
        edges_sorted.push_back(old_to_new_mapping.at(edges.at(j)));
      }
    }
    sorted = true;
  }

private:
  std::vector<unsigned> edge_index;
  std::vector<unsigned> edges;

  std::vector<unsigned> edge_index_sorted;
  std::vector<unsigned> edges_sorted;

  std::vector<std::vector<double>> nodes;
  int node_features{0};
  bool sorted = false;

public:
  [[nodiscard]] int getNodeFeatures() const;
};

#endif