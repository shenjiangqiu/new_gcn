data=["pump","cite","reddit","cora"]
data_fullname=[i+".graph" for i in data]
from genericpath import exists
from os import mkdir, path, system, times
from os.path import expanduser
import shutil
home=expanduser("~")
data_dir=path.abspath(home+"/.gcndata/")
target_path=[data_dir+"/"+b for b in data_fullname]

for item in data_fullname:
    if path.exists(data_dir+"/"+item):
        print("file: {},exists".format(data_dir+"/"+item))

        continue
    else:
        if not (path.exists(item)):
            print("downloading: {}".format(item))
            system("wget https://thesjq.com:8999/"+item+" -O "+item)
        if not path.exists(data_dir):
            mkdir(data_dir)
        shutil.move(item,data_dir+"/"+item)
