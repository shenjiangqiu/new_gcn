import re
import workloads_defs
import subprocess
workloads = workloads_defs.workloads
cycle = re.compile(r"cycle: (.*)")
total_agg = re.compile(r"total_agg: (.*)")
total_rounds_in_agg = re.compile(r"total_rounds_in_agg: (.*)")
do_agg = re.compile(r"do_agg: (.*)")
agg_ops = re.compile(r"agg_ops: (.*)")
total_input_windows = re.compile(
    r"global_definitions.total_input_windows: (.*)")
total_edges = re.compile(r"global_definitions.total_edges: (.*)")
hop_histo = re.compile(r"hop_histo:\n((?:.*\n)+)Interface", flags=re.MULTILINE)
all_res = [cycle,
           total_agg,
           total_rounds_in_agg,
           do_agg,
           agg_ops,
           total_input_windows,
           total_edges]
# all_res = [cycle,total_rounds_in_agg,total_input_windows]
all_res = [cycle]
all_res = [total_rounds_in_agg]

out_dir = "results/"
cmds = []
dividers = []
for scheduling_method in ["-enable-ideal-selection", "enable-sequential-selection", ""]:
    # for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:
    

        for buffer_size in [1048576, 1048576*2, 1048576*4]:
            # for divider in [2, 4, 8, 16][0:1]:

            if scheduling_method == "":
                dividers = [2, 4, 8, 16]
            else:
                dividers = [2]
            for divider in dividers:
                print(f"{scheduling_method}_{buffer_size}_{divider} ")

for graph in workloads:
    print(f"\n{graph}:file\n")

    for scheduling_method in ["-enable-ideal-selection", "enable-sequential-selection", ""]:
        # for buffer_size in [1048576, 1048576*2, 1048576*4][1:2]:
        

            for buffer_size in [1048576, 1048576*2, 1048576*4]:
                # for divider in [2, 4, 8, 16][0:1]:

                if scheduling_method == "":
                    dividers = [2, 4, 8, 16]
                else:
                    dividers = [2]
                for divider in dividers:
                    # print(f"{scheduling_method}_{buffer_size}_{divider} ")
                    file=f"{graph}.{buffer_size}.{divider}.{scheduling_method}.sched.out"
                    proc = subprocess.Popen(['tail', '-n', "100", file], stdout=subprocess.PIPE)
                    txt=proc.stdout.read().decode("utf-8")
                    for r in all_res:
                        if len(r.findall(txt)) != 0:
                            print("{}".format(r.findall(txt)[-1]), end=" ")
                        else:
                            print("not_ready", end=" ")


