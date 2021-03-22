import os


workloads=["acm", "blogcatalog",   "coauthor_cs",  "pubmed",  \
          "amazon_cs", "citeseer_gog", "coauthor_phy", "flickr", \
          "amazon_photo", "citeseer",  "cora_full",  "cora_ml_gog"] 

for  graph  in workloads:
    cmd = "nohup ./gcn_sim -input=131072 -output=4194304 \
        -edge=2097152 -agg=4194304 -aggCores=512 \
        -systolic-rows=32 -systolic-cols=128 \ "
    cmd = cmd + " -graph-name=" + graph      
    cmd = cmd + " -dram-name=HBM-config.cfg -model=gsc \
        -ignore-neighbor=0 -ignore-self=0  -enable-feature-sparsity=1\
         -feature-sparse-rate0=0.9  -feature-sparse-rate1=0.9  -feature-sparse-rate2=0.9 \
        -mem-sim=ramulator  -dram-freq=0.5 >"
    cmd  = cmd + graph+"_ramuV5.txt&"
    print(cmd)
    os.system(cmd)
