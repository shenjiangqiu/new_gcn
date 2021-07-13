import re
import workloads_defs
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
all_res = [cycle]
for g in workloads:
    print(f"{g}:file")
    for div in [2, 4, 8, 16]:
        for buf in [1048576, 1048576*2, 1048576*4]:
            txt = open(f"{g}.{buf}.{div}.sched.out").read()

            for r in all_res:
                if len(r.findall(txt)) != 0:
                    print("{}".format(r.findall(txt)[-1]), end=" ")
                else:
                    print("not_ready", end=" ")
            # print(f"{g}.{buf}.{div}")
