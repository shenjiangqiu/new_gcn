#include "System.h"
#include "utils/Options.h"
#include <fmt/format.h>
#include <fstream>
#include <graph.h>
#include <iostream>
using namespace Minisat;
static IntOption inputSize("system", "input", "the input buffer size(Byte)");
static IntOption outputSize("system", "output", "the output buffer size(Byte)");
static IntOption edgeSize("system", "edge", "the edge buffer size(Byte)");
static IntOption aggSize("system", "agg", "the agg buffer size(Byte)");

static IntOption aggCores("system", "aggCores", "the agg buffer size(Byte)");
static IntOption systolic_rows("system", "systolic-rows",
                               "the agg buffer size(Byte)");
static IntOption systolic_cols("system", "systolic-cols",
                               "the agg buffer size(Byte)");
static StringOption graph_name("system", "graph-name", "the name of graph");
int main(int argc, char **argv) {

  Minisat::parseOptions(argc, argv, false);
  std::vector<int> node_sizes;
  System m_system(inputSize, edgeSize, aggSize, outputSize, aggCores,
                  systolic_rows, systolic_cols, (std::string)graph_name,
                  node_sizes,
                  slide_window_set_iterator(__gnu_cxx::__normal_iterator(),
                                            __gnu_cxx::__normal_iterator(),
                                            __gnu_cxx::__normal_iterator(),
                                            __gnu_cxx::__normal_iterator()));

  return 0;
}