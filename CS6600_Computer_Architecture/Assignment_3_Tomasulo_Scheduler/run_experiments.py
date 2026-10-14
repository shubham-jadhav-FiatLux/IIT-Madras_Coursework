import subprocess,re,json
res={}
for tr in ('gcc','perl'):
    for N in (2,4,8):
        for S in (8,16,32,64,128,256):
            o=subprocess.run(['./ooosim',str(N),str(S),f'{tr}_trace.txt'],capture_output=True,text=True).stdout
            ipc=float(re.search(r'IPC\s*=\s*([\d.]+)',o).group(1)); cyc=int(re.search(r'number of cycles\s*=\s*(\d+)',o).group(1))
            isd=[int(m) for m in re.findall(r'IS\{\d+,(\d+)\}',o)]; idd=[int(m) for m in re.findall(r'ID\{\d+,(\d+)\}',o)]
            res[f'{tr},{N},{S}']=dict(ipc=ipc,cycles=cyc,is_avg=sum(isd)/len(isd),id_avg=sum(idd)/len(idd))
json.dump(res,open('ooo_sweep.json','w'),indent=0)
for tr in ('gcc','perl'):
    for N in (2,4,8): print(tr,N,[res[f'{tr},{N},{S}']['ipc'] for S in (8,16,32,64,128,256)],'IS avg',[round(res[f'{tr},{N},{S}']['is_avg'],1) for S in (8,16,32,64,128,256)])
