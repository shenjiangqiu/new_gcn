import graph

import workloads_defs

workloads = workloads_defs.workloads
for i in workloads:
    file = i+'.graph'
    mgraph = graph.Graph(file)
    feature_size = mgraph.feature_size
    input_size = int(131072/(feature_size*4*2))
    output_size = int(16777216/(feature_size*4*2))

    mgraph.setup_hardware(max_input_num=input_size, max_output_num=output_size)
    mgraph.setup_window(divider=input_size, max_queue_size=32768,dynamic_bloom=True,bloom_entries=32768)
    print(file)
    mgraph.print_result()
