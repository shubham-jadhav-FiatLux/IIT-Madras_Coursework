import subprocess,re,json
def run(args):
    o=subprocess.run(['./bpsim']+args,capture_output=True,text=True).stdout
    return float(re.search(r'misprediction rate:\s*([\d.]+)%',o).group(1))
res={}
for tr in ('gcc','jpeg'):
    f=f'{tr}_trace.txt'
    for m in range(7,17): res[f'bimodal,{tr},{m}']=run(['bimodal',str(m),f])
    for n in range(2,17,2):
        for m in range(max(7,n),17): res[f'gshare,{tr},{m},{n}']=run(['gshare',str(m),str(n),f])
    for n in (0,1,3,5,7,9):
        for m in range(max(7,n),17): res[f'gshare,{tr},{m},{n}']=run(['gshare',str(m),str(n),f])
json.dump(res,open('bp_sweep.json','w'))
for tr in ('gcc','jpeg'):
    print(tr,'bimodal',[ (m,res[f'bimodal,{tr},{m}']) for m in range(7,17)])
    best=min((v,k) for k,v in res.items() if k.startswith(f'gshare,{tr}') and int(k.split(',')[2])<=16)
    print(tr,'best gshare overall',best)
