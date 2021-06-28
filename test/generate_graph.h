//
// Created by Jiangqiu shen on 5/6/21.
//

#ifndef GCN_SIM_GENERATE_GRAPH_H
#define GCN_SIM_GENERATE_GRAPH_H
#include "fstream"
#include "string.h"
void generate_test_graph();
inline void generate_small_graph() {
  auto str = "f 10\n"
             "1 2 3\n"
             "2 3\n"
             "3\n"
             "0";
  auto f = std::ofstream("test.graph");
  f.write(str, strlen(str));
  f.close();
}
#endif // GCN_SIM_GENERATE_GRAPH_H
