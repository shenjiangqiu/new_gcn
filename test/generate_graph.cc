//
// Created by Jiangqiu shen on 5/6/21.
//
#include "generate_graph.h"

void generate_test_graph() {
  auto str = "f 10\n"
             "1 2 3 4 7 13 14\n"
             "4 9 13\n"
             "8 15\n"
             "4 9 10\n"
             "5\n"
             "6\n"
             "7\n"
             "8\n"
             "9\n"
             "1\n"
             "1\n"
             "12 11\n"
             "13 1 2 3 4 5 9 12\n"
             "1\n"
             "15 12\n"
             "1";
  auto f = std::ofstream("test.graph");
  f.write(str, strlen(str));
  f.close();
}
