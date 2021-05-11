//
// Created by sjq on 1/8/21.
//

#include "globals.h"
#include "utils/Options.h"
globals global_definitions;

namespace config {
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
                                 "cora");
Minisat::StringOption dram_name("system", "dram-name", "the name of dram",
                                "DDR4-config.cfg");
Minisat::BoolOption debug("system", "debug", "if enable debug", false);

Minisat::StringOption model("system", "model", "the model definition file",
                            "gcn");
Minisat::IntOption ignore_neighbor("system", "ignore-neighbor",
                                   "how many feature are ignored by neighbor",
                                   0, Minisat::IntRange(0, INT32_MAX));
Minisat::IntOption ignore_self("system", "ignore-self",
                               "how many feature are ignored by neighbor", 0,
                               Minisat::IntRange(0, INT32_MAX));
Minisat::DoubleOption core_freq("system", "core-freq",
                                "the core frequency in GHZ", 1);
Minisat::DoubleOption dram_freq("system", "dram-freq",
                                "the dram frequency in GHZ,Note that, It's "
                                "working frequency not data frequency!!",
                                1);
Minisat::StringOption mem_sim("system", "mem-sim",
                                  "memory simulator",
                                  "ramulator");

Minisat::IntOption enable_feature_sparsity("system", "enable-feature-sparsity", "if enable feature sparsity", 0);
Minisat::DoubleOption feature_sparse_rate0("system", "feature-sparse-rate0",
                                "Feature sparse rate layer 0", 0);

Minisat::DoubleOption feature_sparse_rate1("system", "feature-sparse-rate1",
                                "Feature sparse rate layer 1", 0);
                                
Minisat::DoubleOption feature_sparse_rate2("system", "feature-sparse-rate2",
                                "Feature sparse rate layer 2", 0);
Minisat::BoolOption enable_valid_node_only("system","enable-valid-node-only","enable-valid-node-only",false);
Minisat::BoolOption enable_dense_window("system","enable-dense-window","enable-dense-window",false);

} // namespace config