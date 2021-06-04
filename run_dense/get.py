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
do_aggregate=re.compile(r"do_aggregate : (.*)")
do_systolic=re.compile(r"do_systolic : (.*)")
total_Aggregator_idle_waiting_edge=re.compile(r"total_Aggregator_idle_waiting_edge : (.*)")
total_Aggregator_idle_waiting_input=re.compile(r"total_Aggregator_idle_waiting_input : (.*)")
total_aggregate_op=re.compile(r"total_aggregate_op : (.*)")
total_cycles=re.compile(r"total_cycles : (.*)")
total_edgeBuffer_idle_cycles=re.compile(r"total_edgeBuffer_idle_cycles : (.*)")
total_handle_edges=re.compile(r"total_handle_edges : (.*)")
total_idle_waiting_agg_write=re.compile(r"total_idle_waiting_agg_write : (.*)")
total_idle_waiting_out=re.compile(r"total_idle_waiting_out : (.*)")
total_inputBuffer_idle_cycles=re.compile(r"total_inputBuffer_idle_cycles : (.*)")
total_input_windows=re.compile(r"total_input_windows : (.*)")
total_mac_in_systolic_array=re.compile(r"total_mac_in_systolic_array : (.*)")
total_read_edge_traffic=re.compile(r"total_read_edge_traffic : (.*)")
total_read_input_traffic=re.compile(r"total_read_input_traffic : (.*)")
total_real_edge_idle=re.compile(r"total_real_edge_idle : (.*)")
total_real_input_idle=re.compile(r"total_real_input_idle : (.*)")
total_systolicArray_idle_waiting_agg_read=re.compile(r"total_systolicArray_idle_waiting_agg_read : (.*)")

all_res=[
do_aggregate
,do_systolic
,total_Aggregator_idle_waiting_edge
,total_Aggregator_idle_waiting_input
,total_aggregate_op
,total_cycles
,total_edgeBuffer_idle_cycles
,total_handle_edges
,total_idle_waiting_agg_write
,total_idle_waiting_out
,total_inputBuffer_idle_cycles
,total_input_windows
,total_mac_in_systolic_array
,total_read_edge_traffic
,total_read_input_traffic
,total_real_edge_idle
,total_real_input_idle
,total_systolicArray_idle_waiting_agg_read
]
all_res=[total_cycles,do_aggregate,total_aggregate_op,total_handle_edges,total_input_windows]

for g in workloads:
    
    txt = open(f"{g}.dense.out").read()
    print("\n"+g+":file")
    for r in all_res:
        if len(r.findall(txt))!=0:
            print("{}".format(r.findall(txt)[-1]),end=" ")
        else:
            print("not ready",end=" ")
