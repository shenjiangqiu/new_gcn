#!/bin/python3
# %%
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import sys

file_name = sys.argv[1]
f = open(file_name)
f_rec = open(sys.argv[2])

line = f.readline()
i = 0
fig, ax = plt.subplots()
plt.rcParams["figure.figsize"] = [100, 100]

while len(line) > 0:
    array = np.array(line.split(), dtype=int)
    array = np.append(array, i)
    # print(array)
    ax.plot([i for _ in range(len(array))], array, 'r.', 1)
    line = f.readline()
    i = i + 1

line = f_rec.readline()
while len(line) > 0:
    vec = line.split()

    a = np.array(vec[:5], dtype=int)
    color = 'bgrcmyk'[a[4]]
    line_width = 10 if vec[5] == "true" else 5 if vec[6] == "true" else 1
    rec = patches.Rectangle((a[0], a[1]), a[2] - 1, a[3] - 1, edgecolor=color, facecolor='none', linewidth=line_width)
    ax.add_patch(rec)
    line = f_rec.readline()
plt.savefig("fig")
