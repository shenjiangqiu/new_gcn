import re
workloads = ["acm_graph_data",            "coauthor_cs_graph_data",   "dblp_graph_data",
             "amazon_cs_graph_data",      "coauthor_phy_graph_data",  "flickr_graph_data",
             "amazon_photo_graph_data",   "cora_full_graph_data",                    "pubmed_gog",
             "blogcatalog_graph_data",    "cora_gog",                                "pubmed",
             "cite",                      "cora",                                  "pubmed_graph_data",
             "citeseer_full_graph_data",  "cora_graph_data",            "pump",
             "citeseer_gog",              "cora_ml_gog",                        "reddit",
             "citeseer",                  "cora_ml_graph_data",                          "test",
             "citeseer_graph_data",       "dblp.gog",                              "uai_graph_data"]

cycle = re.compile(r"cycle: (.*)")
total_agg = re.compile(r"total_agg: (.*)")
total_rounds_in_agg = re.compile(r"total_rounds_in_agg: (.*)")
do_agg = re.compile(r"do_agg: (.*)")
agg_ops = re.compile(r"agg_ops: (.*)")
total_input_windows = re.compile(
    r"global_definitions.total_input_windows: (.*)")
total_edges = re.compile(r"global_definitions.total_edges: (.*)")

all_res = [cycle,
           total_agg,
           total_rounds_in_agg,
           do_agg,
           agg_ops,
           total_input_windows,
           total_edges]
for g in workloads:

    txt = open(f"{g}.fast_sched.out").read()
    print("\n"+g+":file")
    for r in all_res:
        if len(r.findall(txt)) != 0:
            print("{}".format(r.findall(txt)[-1]), end=" ")
        else:
            print("not ready", end=" ")
