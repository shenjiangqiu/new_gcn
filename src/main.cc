#include "System.h"
#include "spdlog/spdlog.h"
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
static StringOption graph_name("system", "graph-name", "the name of graph",
                               "test");
static StringOption dram_name("sym", "dram-name", "the name of dram",
                              "DDR4-config.cfg");
static BoolOption debug("system", "debug", "if enable debug", false);
int main(int argc, char **argv) {

  Minisat::parseOptions(argc, argv, false);
  // TODO: should be read gcn number
  // the features dimension for each layer
  if (debug) {
    spdlog::set_level(spdlog::level::debug);
  }
  std::vector<int> node_sizes = {10, 20, 10};

  System m_system(inputSize, edgeSize, aggSize, outputSize, aggCores,
                  systolic_rows, systolic_cols, (std::string)graph_name,
                  node_sizes, (std::string)dram_name);
  m_system.run();

  return 0;
}