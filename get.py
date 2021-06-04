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


workloads=["Graph_saint_reddit"]

do_aggregate=re.compile(r".*do_aggregate : (.*)")
do_systolic=re.compile(r".*do_systolic : (.*)")
total_Aggregator_idle_waiting_edge=re.compile(r".*total_Aggregator_idle_waiting_edge : (.*)")
total_Aggregator_idle_waiting_input=re.compile(r".*total_Aggregator_idle_waiting_input : (.*)")
total_aggregate_op=re.compile(r".*total_aggregate_op : (.*)")
total_cycles=re.compile(r".*total_cycles : (.*)")
total_edgeBuffer_idle_cycles=re.compile(r".*total_edgeBuffer_idle_cycles : (.*)")
total_handle_edges=re.compile(r".*total_handle_edges : (.*)")
total_idle_waiting_agg_write=re.compile(r".*total_idle_waiting_agg_write : (.*)")
total_idle_waiting_out=re.compile(r".*total_idle_waiting_out : (.*)")
total_inputBuffer_idle_cycles=re.compile(r".*total_inputBuffer_idle_cycles : (.*)")
total_input_windows=re.compile(r".*total_input_windows : (.*)")
total_mac_in_systolic_array=re.compile(r".*total_mac_in_systolic_array : (.*)")
total_read_edge_traffic=re.compile(r".*total_read_edge_traffic : (.*)")
total_read_input_traffic=re.compile(r".*total_read_input_traffic : (.*)")
total_real_edge_idle=re.compile(r".*total_real_edge_idle : (.*)")
total_real_input_idle=re.compile(r".*total_real_input_idle : (.*)")
total_systolicArray_idle_waiting_agg_read=re.compile(r".*total_systolicArray_idle_waiting_agg_read : (.*)")

total_read_traffic=re.compile(r".*total_read_traffic: (.*)")
total_write_traffic=re.compile(r".*total_write_traffic: (.*)")
total_read_traffic_input=re.compile(r".*total_read_traffic_input: (.*)")
total_read_traffic_edge=re.compile(r".*total_read_traffic_edge: (.*)")


cycle=re.compile(r".*total_cycles : (.*)")





for g in workloads:
    
    txt = open(f"{g}.cont.out").read()
    print(g)
    print(do_aggregate.findall(txt)[-1])
    print(do_systolic.findall(txt)[-1])
    print(total_Aggregator_idle_waiting_edge.findall(txt)[-1])
    print(total_Aggregator_idle_waiting_input.findall(txt)[-1])
    print(total_aggregate_op.findall(txt)[-1])
    print(total_cycles.findall(txt)[-1])
    print(total_edgeBuffer_idle_cycles.findall(txt)[-1])
    print(total_handle_edges.findall(txt)[-1])
    print(total_idle_waiting_agg_write.findall(txt)[-1])
    print(total_idle_waiting_out.findall(txt)[-1])
    print(total_inputBuffer_idle_cycles.findall(txt)[-1])
    print(total_input_windows.findall(txt)[-1])
    print(total_mac_in_systolic_array.findall(txt)[-1])
    print(total_read_edge_traffic.findall(txt)[-1])
    print(total_read_input_traffic.findall(txt)[-1])
    print(total_real_edge_idle.findall(txt)[-1])
    print(total_real_input_idle.findall(txt)[-1])
    print(total_systolicArray_idle_waiting_agg_read.findall(txt)[-1])


    print(cycle.findall(txt)[-1])

    print(total_read_traffic.findall(txt)[-1])
    print(total_write_traffic.findall(txt)[-1])
    print(total_read_traffic_input.findall(txt)[-1])
    print(total_read_traffic_edge.findall(txt)[-1])

