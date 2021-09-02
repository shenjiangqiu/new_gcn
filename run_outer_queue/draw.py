from matplotlib import pyplot as plt

import os
import re
import numpy as np
import sys
agg = re.compile(r".*agg_buffer: (.*) (.*)")
edges = re.compile(r".*edge_buffer: (.*) (.*)")


def draw_curve(filename):
    with open(filename) as f:
        txt = f.read()
        agg_data = np.array(agg.findall(txt),dtype=float)
        edge_data = np.array(edges.findall(txt),dtype=float)
        print(agg_data.shape)
        agg_data=agg_data[:,0]/agg_data[:,1]
        edge_data=edge_data[:,0]/edge_data[:,1]


        fig, ax = plt.subplots()
        x = range(len(agg_data))
        ax.plot(x, agg_data,".-",label="agg")
        ax.plot(x, edge_data,".-",c="r",label="edge")
        ax.legend()
        fig.savefig("edge_agg.png")

if __name__ == "__main__":
    draw_curve(sys.argv[1])