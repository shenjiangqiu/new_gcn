./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=2097152 -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name=cora -dram-name=HBM-config.cfg -model=gcn >2Magg.txt

./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=4194304 -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name=cora -dram-name=HBM-config.cfg -model=gcn >4Magg.txt

./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=8388608 -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name=cora -dram-name=HBM-config.cfg -model=gcn >8Magg.txt

./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=16777216 -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name=cora -dram-name=HBM-config.cfg -model=gcn >16Magg.txt

./gcn_sim -input=131072 -output=4194304 -edge=2097152 -agg=33554432 -aggCores=512 -systolic-rows=32 -systolic-cols=128 -graph-name=cora -dram-name=HBM-config.cfg -model=gcn >32Magg.txt

