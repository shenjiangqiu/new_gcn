
from multiprocessing import Pool
import subprocess
import workloads_defs

#workloads1=["Graph_saint_flickr", "Graph_saint_ogbn-arxiv",  "Graph_saint_ogbn-arxiv_undirected", "Graph_saint_ppi",  "Graph_saint_ppi-large",  "Graph_saint_yelp"]
#workloads=workloads[0:1]
# large graph
#workloads=[ "Graph_saint_amazon","Graph_saint_ogbn-products","Graph_saint_reddit","reddit", "Graph_saint_yelp"]

workloads=workloads_defs.workloads
#workloads=workloads[0:1]
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


for graph in workloads:
    cmd = f"./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size=4194304 -agg=16777216 \
        -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
        -model=gsc -ignore-neighbor=0 -ignore-self=0  \
            -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched>{graph}.sched.out"
    print(cmd)
    cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)


with Pool(int(10)) as p:
    p.map(run_task, cmds)
