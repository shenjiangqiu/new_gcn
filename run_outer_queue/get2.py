import re
import workloads_defs
import subprocess

large_graphs = ["amazon_cs_graph_data", "blogcatalog_graph_data", "coauthor_cs_graph_data",
                "coauthor_phy_graph_data", "flickr_graph_data", "Graph_saint_ogbn-arxiv", "Graph_saint_ppi", "uai_graph_data"]
large_graphs = ["amazon_cs_graph_data", "blogcatalog_graph_data",
                "coauthor_cs_graph_data", "coauthor_phy_graph_data", "flickr_graph_data"]
smart_bfs_diviers = [0.01, 0.05, 0.1, 0.2]


workloads = workloads_defs.workloads
cycle = re.compile(r"cycle: (.*)")
total_cycle = re.compile(r"total_cycles : (.*)")
total_agg = re.compile(r"total_agg: (.*)")
total_rounds_in_agg = re.compile(r"total_rounds_in_agg: (.*)")
do_agg = re.compile(r"do_agg: (.*)")
agg_ops = re.compile(r"agg_ops: (.*)")
total_input_windows = re.compile(
    r"global_definitions.total_input_windows: (.*)")
total_edges = re.compile(r"global_definitions.total_edges: (.*)")
hop_histo = re.compile(r"hop_histo:\n((?:.*\n)+)Interface", flags=re.MULTILINE)
all_res = [cycle,
           total_agg,
           total_rounds_in_agg,
           do_agg,
           agg_ops,
           total_input_windows,
           total_edges]
# all_res = [cycle,total_rounds_in_agg,total_input_windows]
all_res = [cycle, total_cycle]

out_dir = "results/"
cmds = []
dividers = []
for graph in large_graphs:
    print(f"\n{graph}:file\n")

    for scheduling_method in ["sliding", "dense"]:
        # for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:

        file = f"{graph}.{scheduling_method}.sched.out"
        proc = subprocess.Popen(
            ['tail', '-n', "100", file], stdout=subprocess.PIPE)
        txt = proc.stdout.read().decode("utf-8")
        for r in [total_cycle]:
            if len(r.findall(txt)) != 0:
                print(f" {(r.findall(txt)[-1])}", end=" ")
            else:
                print(f" not_ready", end=" ")
    for scheduling_method in ["seq", "bfs", "bdfs"]:

        file = f"{graph}.{scheduling_method}.sched.out"
        proc = subprocess.Popen(
            ['tail', '-n', "100", file], stdout=subprocess.PIPE)
        txt = proc.stdout.read().decode("utf-8")
        for r in [cycle]:
            if len(r.findall(txt)) != 0:
                print(f" {(r.findall(txt)[-1])}", end=" ")
            else:
                print(f" not_ready", end=" ")
    smart_bfs_dividers = [0.01, 0.05, 0.1, 0.2]
    for divider in smart_bfs_dividers:
        file = f"{graph}.smartbfs.{divider}.sched.out"
        proc = subprocess.Popen(
            ['tail', '-n', "100", file], stdout=subprocess.PIPE)
        txt = proc.stdout.read().decode("utf-8")
        for r in [cycle]:
            if len(r.findall(txt)) != 0:
                print(f"{r.findall(txt)[-1]}", end=" ")
            else:
                print(f" not_ready", end=" ")
print("")
