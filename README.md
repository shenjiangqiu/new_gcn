# new_gcn

**A hy_gcn simulator**

To build the code:
```
git clone --recursive https://github.com/shenjiangqiu/new_gcn
cd new_gcn
mkdir build&&cmake ..&&make
```



HBM is copied from https://github.com/gthparch/HBM

HBM Simulator based on DRAMSim2; a small source-code change with HBM configuration

1 Build------------------------------------------------------------------------

type:
$ make

2 Usage------------------------------------------------------------------------

DRAMSim2 can be built as a dynamic shared library which is convenient for
connecting it to CPU simulators or other custom front ends. A MemorySystem
object encapsulates the functionality of the memory system. 


