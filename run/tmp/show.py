import numpy
import matplotlib.pyplot as plt

f = open('handle_edges.txt')
edges = f.read()
f.close()

f = open('input_windows.txt')
windows = f.read()
f.close()

f = open('names.txt')
names = f.read()
f.close()

windows_list = list(windows.split())
edges_list = list(edges.split())
names_list = list(names.split())

num = len(windows_list)
rate = numpy.zeros((1,num))
for i, x in enumerate(windows_list):
     print(names_list[i],"  ", float(edges_list[i])/float(windows_list[i]))


edges = numpy.array(edges_list)
windows = numpy.array(windows_list)
edges_f= edges.astype('float')
windows_f = windows.astype('float')
rate = numpy.divide(edges_f, windows_f) 
x = numpy.arange(num)
fig=plt.figure()
#plt.plot(x,rate)
#plt.show()
fig = plt.figure()
h = plt.bar(names_list,rate)
plt.subplots_adjust(bottom=0.3)
xticks_pos = [0.65*patch.get_width() + patch.get_xy()[0] for patch in h]
plt.xticks(xticks_pos, names_list,  ha='right', rotation=45)
plt.title("Num of Edges in a Input Window")
plt.savefig('rate.jpg')

