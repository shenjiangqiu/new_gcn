import os

from multiprocessing import Pool
import subprocess
workloads = ["acm", "blogcatalog",   "coauthor_cs",  "pubmed",
             "amazon_cs", "citeseer_gog", "coauthor_phy", "flickr",
             "amazon_photo", "citeseer",  "cora_full",  "cora_ml_gog"
             "Graph_saint_ogbn-arxiv", "Graph_saint_ogbn-arxiv_undirected",
             "Graph_saint_flick", "Graph_saint_ppi-large", "Graph_saint_ppi",
             "Graph_saint_yelp"]
workloads = ["acm_graph_data",            "coauthor_cs_graph_data",   "dblp_graph_data",                    "Graph_saint_reddit",
             "amazon_cs_graph_data",      "coauthor_phy_graph_data",  "flickr_graph_data",                  "Graph_saint_yelp",
             "amazon_photo_graph_data",   "cora_full_graph_data",     "Graph_saint_amazon",                 "pubmed_gog",
             "blogcatalog_graph_data",    "cora_gog",                 "Graph_saint_flickr",                 "pubmed",
             "cite",                      "cora",                     "Graph_saint_ogbn-arxiv",             "pubmed_graph_data",
             "citeseer_full_graph_data",  "cora_graph_data",          "Graph_saint_ogbn-arxiv_undirected",  "pump",
             "citeseer_gog",              "cora_ml_gog",              "Graph_saint_ogbn-products",          "reddit",
             "citeseer",                  "cora_ml_graph_data",       "Graph_saint_ppi",                    "test",
             "citeseer_graph_data",       "dblp.gog",                 "Graph_saint_ppi-large",              "uai_graph_data"]
workloads = workloads[0:10]
#workloads1=["Graph_saint_flickr", "Graph_saint_ogbn-arxiv",  "Graph_saint_ogbn-arxiv_undirected", "Graph_saint_ppi",  "Graph_saint_ppi-large",  "Graph_saint_yelp"]

# large graph
#workloads=[ "Graph_saint_amazon","Graph_saint_ogbn-products","Graph_saint_reddit","reddit", "Graph_saint_yelp"]

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
            -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched>{graph}.fast_sched.out"
    print(cmd)
    print(cmd.split())
    cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)

with Pool(int(4)) as p:
    p.map(run_task, cmds)
