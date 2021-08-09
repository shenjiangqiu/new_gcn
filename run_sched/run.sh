nohup ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=cora_gog\
        -dram-name=HBM-config.cfg -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=ramulator  -dram-freq=0.5 > cora_gog_ramu.txt&

nohup ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=pubmed\
        -dram-name=HBM-config.cfg -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=ramulator  -dram-freq=0.5 > cora_ml_ramu.txt&


nohup ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=dblp.gog\
        -dram-name=HBM-config.cfg -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=ramulator  -dram-freq=0.5 > dblp.gog_ramu.txt&




