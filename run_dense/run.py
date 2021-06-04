#!/usr/bin/python3
import argparse
from multiprocessing import Pool
import subprocess
import sys
import os
def run():
    parser = argparse.ArgumentParser(description="Run the benchmark for gcn")
    parser.add_argument("--bin", dest="bin", action="store",
                        default="./new_gcn", help="the path of gcn")

    parser.add_argument("--num-core", dest="num_cores", default=4,
                        help="at most use how many cpu cores to run this task")
    parser.add_argument("--args", dest="args", default=None)
    parser.add_argument("--graph", dest="graph", action="store",
                        default="pump", help="the name of graph")

    args = parser.parse_args()

    numcores = args.num_cores
    path = os.path.abspath(os.path.expanduser(args.bin))
    if not os.path.exists(path) or args.bin=="":
        print("bin:{} doesn't exist!".format(path))
        return -1
    if args.graph=="":
        print("graph is empty")
        return -1
    graph = os.path.abspath(os.path.expanduser("~/.gcndata/"+args.graph+".graph"))
    if not os.path.exists(graph):
        print("graph:{} doesn't exist".format(graph))
        return -1



    def run_task(command):
        subprocess.run(command, shell=True)
    base_command="./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=reddit\
        -dram-name=HBMSystemLegacy.ini -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=dramsim2 -dram-freq=0.5"
    def command=["{} -dram-name={}".format(base_command,"")]
    print("commands are {}".format(commands))
    print("num_cores is  {}".format(numcores))

    if not os.path.exists(minisat_path):
        print("minisat not exits in {}".format(minisat_path))
        exit(-1)

    with Pool(int(numcores)) as p:
        p.map(run_task, commands)
