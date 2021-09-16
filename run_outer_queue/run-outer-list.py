
from multiprocessing import Pool
import subprocess
import workloads_defs
large_graphs = ["amazon_cs_graph_data", "blogcatalog_graph_data",
                "coauthor_cs_graph_data", "coauthor_phy_graph_data", "flickr_graph_data"]
# large_graphs = ["amazon_cs_graph_data"]

large_graphs_full = [a+'.graph' for a in large_graphs]
smart_bfs_diviers = [0.01, 0.05, 0.1, 0.2]


#workloads1=["Graph_saint_flickr", "Graph_saint_ogbn-arxiv",  "Graph_saint_ogbn-arxiv_undirected", "Graph_saint_ppi",  "Graph_saint_ppi-large",  "Graph_saint_yelp"]
# workloads=workloads[0:1]
# large graph
#workloads=[ "Graph_saint_amazon","Graph_saint_ogbn-products","Graph_saint_reddit","reddit", "Graph_saint_yelp"]
# large_graphs = [ "Graph_saint_ogbn-arxiv", "Graph_saint_ppi", "uai_graph_data"]
# large_graphs_full = ["Graph_saint_ogbn-arxiv.graph", "Graph_saint_ppi.graph", "uai_graph_data.graph"]

workloads = workloads_defs.workloads
# workloads = workloads[0:1]
mem_sim = "ramulator"
mem = "HBM-config.cfg"  # ramulator

#mem_sim = "dramsim3"
# mem = "HBM2_8Gb_x128.ini"    #DRAMsim3

#mem_sim = "dramsim2"
# mem = "HBMSystemLegacy.ini" #DRAMsim2

valid_node_only = " "
#valid_node_only = "  -enable-valid-node-only  "

out_dir = "results/"
# enable_ideal_hash = ""
# enable_ideal_hash = ""
enable_ideal_hash = "-enable-ideal-hash1 -enable-reduced-entry-hash2 -enable-placeholder-hash2"

cmds = []
dividers = []
for scheduling_method in ["-enable-sequential-selection"]:
    # for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:
    for buffer_size in [1048576*4]:

        # for divider in [2, 4, 8, 16][0:1]:
        for graph in large_graphs:
            # # sliding
            # cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
            #     -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
            #     -model=gsc -ignore-neighbor=0 -ignore-self=0  \
            #         -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 \
            #                 -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection >{graph}.sliding.sched.out 2>&1"
            # print(cmd)
            # cmds.append(cmd)

            # dense
            # cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
            #     -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
            #     -model=gsc -ignore-neighbor=0 -ignore-self=0  \
            #         -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window \
            #                 -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection >{graph}.dense.sched.out 2>&1"
            # print(cmd)
            # cmds.append(cmd)

            # seq
            cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
                -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched \
                            -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection {enable_ideal_hash} >{graph}.seq.sched.out 2>&1"
            print(cmd)
            cmds.append(cmd)

            # bfs

            cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
                -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched \
                            -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection -enable-outer-list -outer-name=lists/{graph}.graph.bfs.30.seqresult\
                               {enable_ideal_hash}  >{graph}.bfs.sched.out 2>&1"
            print(cmd)
            cmds.append(cmd)
            # bdfs
            cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
                -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched \
                            -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection -enable-outer-list {enable_ideal_hash} -outer-name=lists/{graph}.graph.bdfs.30.seqresult >{graph}.bdfs.sched.out 2>&1"
            print(cmd)
            cmds.append(cmd)
            for divider in smart_bfs_diviers:
                cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -hash-table-size={buffer_size} -agg=16777216 \
                -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window -enable-fast-sched \
                            -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection -enable-outer-list {enable_ideal_hash} -outer-name=lists/{graph}.graph.smartbfs.30.5.{divider}.seqresult >{graph}.smartbfs.{divider}.sched.out 2>&1"
                print(cmd)
                cmds.append(cmd)

            print(cmd)
            cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)


print(f"cmds: {cmds}")
with Pool(int(1)) as p:
    p.map(run_task, cmds)
