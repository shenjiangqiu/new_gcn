
from multiprocessing import Pool
import subprocess
import workloads_defs
large_graphs = ["amazon_cs_graph_data.graph", "blogcatalog_graph_data.graph", "coauthor_cs_graph_data.graph", "coauthor_phy_graph_data.graph",
                "flickr_graph_data.graph", "Graph_saint_ogbn-arxiv.graph", "Graph_saint_ppi.graph", "uai_graph_data.graph"]
large_graphs = ["cora"]
#workloads1=["Graph_saint_flickr", "Graph_saint_ogbn-arxiv",  "Graph_saint_ogbn-arxiv_undirected", "Graph_saint_ppi",  "Graph_saint_ppi-large",  "Graph_saint_yelp"]
# workloads=workloads[0:1]
# large graph
#workloads=[ "Graph_saint_amazon","Graph_saint_ogbn-products","Graph_saint_reddit","reddit", "Graph_saint_yelp"]

workloads = workloads_defs.workloads
workloads = workloads[0:1]
mem_sim = "ramulator"
mem = "HBM-config.cfg"  # ramulator

#mem_sim = "dramsim3"
# mem = "HBM2_8Gb_x128.ini"    #DRAMsim3

#mem_sim = "dramsim2"
# mem = "HBMSystemLegacy.ini" #DRAMsim2

valid_node_only = " "
#valid_node_only = "  -enable-valid-node-only  "

out_dir = "results/"
cmds = []

for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:
    for divider in [2, 4, 8, 16][0:1]:
        for graph in large_graphs:
            cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
                -aggCores=4 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched \
                        -short-large-divider={divider} -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection -in-names=cora_0_x.txt,cora_1_x.txt,cora_2_x.txt -mask-names=cora_0_y.txt,cora_1_y.txt " #>{graph}.{buffer_size}.{divider}.sched.seq.out 2>&1"
            print(cmd)
            cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)


with Pool(int(1)) as p:
    p.map(run_task, cmds)
