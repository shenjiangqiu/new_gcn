import re
histo_re=re.compile("output the histo for query\n((?:.*\n)*)Interface.*",re.MULTILINE)


def get_histo(txt):
    out=histo_re.findall(txt)
    if len(out)>=1:
        return out[-1]
    else:
        return "not_found"
import numpy as np
import matplotlib.pyplot as plt
if __name__=="__main__":
    import workloads_defs
    workloads=workloads_defs.workloads
    out_files=[f"{graph}.sched.out" for graph in workloads]
    for file in out_files:
        print(file)
        with open(file) as f:
            txt=f.read()
            out=get_histo(txt)
            
            out=out.split("\n")
            out=[item.split(":") for item in out][0:-1]
            out=np.array(out,dtype=int)
            if len(out)==0:
                continue
            print(out)
            fig,ax=plt.subplots()
            ax.bar(out[:,0],out[:,1])
            fig.savefig(file+".png")
            
