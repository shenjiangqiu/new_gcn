import os

from multiprocessing import Pool
import subprocess
workloads = ["acm_graph_data",            "coauthor_cs_graph_data",   "dblp_graph_data",
             "amazon_cs_graph_data",      "coauthor_phy_graph_data",  "flickr_graph_data",
             "amazon_photo_graph_data",   "cora_full_graph_data",                    "pubmed_gog",
             "blogcatalog_graph_data",    "cora_gog",                                "pubmed",
             "cite",                      "cora",                                  "pubmed_graph_data",
             "citeseer_full_graph_data",  "cora_graph_data",            "pump",
             "citeseer_gog",              "cora_ml_gog",                        "reddit",
             "citeseer",                  "cora_ml_graph_data",                          "test",
             "citeseer_graph_data",       "dblp.gog",                              "uai_graph_data"]

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
    cmd = f"./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=4194304 \
        -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
        -model=gsc -ignore-neighbor=0 -ignore-self=0  \
            -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5>{graph}.cont.out"
    print(cmd)
    cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)


with Pool(int(4)) as p:
    p.map(run_task, cmds)