//
// Created by Jiangqiu shen on 1/19/21.
//

#include "transformer.h"

#include "sstream"
#include "vector"
#include <utility>
#include "cassert"
using std::string;
using std::vector;

vector<string> split(const string &str, const string &delim) {
  vector<string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == string::npos)
      pos = str.length();
    string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

transformer::transformer(std::string input_name, std::string output_name)
    : input_name(std::move(input_name)), output_name(std::move(output_name)) {}
void transformer::translate() {
  std::vector<std::vector<int>> m_graph;

  std::ifstream ifs(input_name);
  std::string line;
  int max = 0;

  while (!ifs.eof()) {
    std::getline(ifs, line);
    auto result = split(line, ",");
    // std::cout<<result[0]<<" "<<result[1]<<std::endl;
    // return;
    auto first = std::stoi(result[0]);
    auto second = std::stoi(result[1]);
    if (first > max) {
      m_graph.resize(first);
      max = first;
    }
    m_graph[first - 1].push_back(second - 1);
  }
  // output
  std::cout<<"finished input, start output"<<std::endl;
  std::ofstream ofs(output_name);
  for (auto &&v : m_graph) {
    assert(!v.empty());
    for (auto &&n : v) {
      ofs << n << " ";
    }
    ofs << "\n";
  }
}
