#./gcn_sim -input=131072 -output=4194304\
# -edge=2097152 -agg=4194304 -aggCores=512\
#  -systolic-rows=32 -systolic-cols=128\
#   -graph-name=pubmed\
#    -dram-name=HBM-config.cfg -model=gcn \
#     -ignore-neighbor=0 -ignore-self=0
./gcn_sim -input=131072 -output=4194304\
 -edge=2097152 -agg=4194304 -aggCores=512\
  -systolic-rows=32 -systolic-cols=128\
   -graph-name=reddit\
    -dram-name=HBM2_8Gb_x128.ini -model=gcn \
     -ignore-neighbor=0 -ignore-self=0
