import numpy as np

# read data from file
data = np.loadtxt('0-x.txt')
# make all value which range from -0.5 to 0.5 to 0
data[np.abs(data) < 2] =0

np.savetxt('0-x-2.txt', data, fmt='%1.3f')
