
from multiprocessing import Pool
import subprocess
graphs=["cora"]

mem_sim = "ramulator"
mem = "HBM-config.cfg"  # ramulator


out_dir = "results/"
cmds = []
# ./gcn_sim -input=131072 -output=4194304 -edge=2097152
for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:
    for divider in [2, 4, 8, 16][0:1]:
        for graph in graphs:
            cmd = f"echo {graph};./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=16777216 \
                -aggCores=4 -systolic-rows=32 -systolic-cols=128 -graph-name={graph} -dram-name={mem} \
                -model=gsc -ignore-neighbor=0 -ignore-self=0  \
                    -enable-feature-sparsity=0  -mem-sim={mem_sim} -dram-freq=0.5 -enable-dense-window  \
                        -short-large-divider={divider} -short-queue-size=100000 -large-queue-size=100000 -enable-sequential-selection -in-names=cora_0_x.txt,cora_1_x.txt,cora_2_x.txt -mask-names=cora_0_y.txt,cora_1_y.txt" #>{graph}.{buffer_size}.{divider}.sched.seq.out 2>&1"
            print(cmd)
            cmds.append(cmd)


def run_task(command):
    subprocess.run(command, shell=True)


with Pool(int(1)) as p:
    p.map(run_task, cmds)
