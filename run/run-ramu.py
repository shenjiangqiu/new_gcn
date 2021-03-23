import os


workloads=["acm", "blogcatalog",   "coauthor_cs",  "pubmed",  \
          "amazon_cs", "citeseer_gog", "coauthor_phy", "flickr", \
          "amazon_photo", "citeseer",  "cora_full",  "cora_ml_gog"] 

mem_sim = "ramulator"
mem = "HBM-config.cfg"      #ramulator

#mem_sim = "dramsim3"
#mem = "HBM2_8Gb_x128.ini"    #DRAMsim3

#mem_sim = "dramsim2"
#mem = "HBMSystemLegacy.ini" #DRAMsim2

for  graph  in workloads:
    cmd = "nohup ./gcn_sim -input=131072 -output=4194304 \
        -edge=2097152 -agg=4194304 -aggCores=512 \
        -systolic-rows=32 -systolic-cols=128 \ "
    cmd = cmd + " -graph-name=" + graph      
    cmd = cmd + " -dram-name=" + mem + " -model=gsc \
        -ignore-neighbor=0 -ignore-self=0  -enable-feature-sparsity=0\
         -feature-sparse-rate0=0  -feature-sparse-rate1=0  -feature-sparse-rate2=0 "
    cmd = cmd + " -mem-sim=" +  mem_sim  + " -dram-freq=0.5 >"
    cmd  = cmd + graph+"_ramuV0.txt&"
    print(cmd)
    os.system(cmd)
