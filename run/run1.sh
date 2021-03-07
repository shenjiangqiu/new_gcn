

if [  $1 = "ramu"  ]
then
    echo "ramlator"
    ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=pump\
        -dram-name=HBM-config.cfg -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=ramulator  -dram-freq=0.5
elif [  $1 = "dram3" ]
then
    echo "Using DRAMsim3"
    ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=pubmed\
        -dram-name=HBM2_8Gb_x128.ini -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
         -mem-sim=dramsim3 -dram-freq=0.5
elif [  $1 = "dram2" ]
then
    echo "Using DRAMSim2"
    ./gcn_sim -input=131072 -output=4194304\
        -edge=2097152 -agg=4194304 -aggCores=512\
        -systolic-rows=32 -systolic-cols=128\
        -graph-name=reddit\
        -dram-name=HBMSystemLegacy.ini -model=gsc \
        -ignore-neighbor=0 -ignore-self=0 \
        -mem-sim=dramsim2 -dram-freq=0.5
else
   echo "invalid memory simulator"
fi
