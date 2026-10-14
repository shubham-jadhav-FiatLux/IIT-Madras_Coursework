import subprocess,json,re,sys,itertools,os,shutil,threading,tempfile
from concurrent.futures import ThreadPoolExecutor
TRACE='gcc_trace.txt'
cache={}
local=threading.local()
def workdir():
    # CACTI writes scratch files into its working directory, so every worker thread gets its own copy
    if not hasattr(local,'d'):
        local.d=tempfile.mkdtemp(); shutil.copy('cacti',local.d); shutil.copy('cache.cfg',local.d); shutil.copy('cache_sim',local.d); shutil.copy('gcc_trace.txt',local.d)
    return local.d
def cacti_ok(size,bs,assoc):
    cmd=['./cacti',str(size),str(bs),'FA' if assoc==size//bs else str(assoc),'45nm','1']
    o=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,cwd=workdir()).stdout
    return 'Access time' in o and 'Cache height x width' in o

def run(l1,a1,bs,vc,l2,a2):
    key=(l1,a1,bs,vc,l2,a2)
    if key in cache: return cache[key]
    out=subprocess.run(['./cache_sim',*map(str,key),TRACE],capture_output=True,text=True,cwd=workdir()).stdout
    g=lambda p: float(re.search(p,out).group(1))
    r=dict(reads=g(r'L1 reads:\s*(\d+)'),rm=g(r'L1 read misses:\s*(\d+)'),writes=g(r'L1 writes:\s*(\d+)'),wm=g(r'L1 write misses:\s*(\d+)'),
           aat=g(r'average access time:\s*([\d.]+)'),edp=g(r'energy-delay product:\s*([\d.]+)'),area=g(r'total area:\s*([\d.]+)'))
    r['mr']=(r['rm']+r['wm'])/(r['reads']+r['writes'])
    r['valid']=cacti_ok(l1,bs,a1) and (l2==0 or cacti_ok(l2,bs,a2))
    cache[key]=r; return r
def many(keys):
    keys=list(dict.fromkeys(keys))
    with ThreadPoolExecutor(8) as ex: res=list(ex.map(lambda k: run(*k),keys))
    return dict(zip(keys,res))
KB=1024
def assocs(size,bs): return {'DM':1,'2-way':2,'4-way':4,'8-way':8,'FA':size//bs}
data={}
# plot 1/2
k1=[(2**e,a,32,0,0,0) for e in range(11,21) for a in assocs(2**e,32).values()]
# plot 3
k3=[(2**e,a,32,0,256*KB,8) for e in range(11,18) for a in assocs(2**e,32).values()]
# plot 4
k4=[(s*KB,4,bs,0,0,0) for s in (1,2,4,8,16,32) for bs in (16,32,64,128)]
# plot 5/6
k5=[(l1*KB,4,32,0,l2*KB,8) for l1 in (1,2,4,8,16,32,64) for l2 in (32,64,128,256,512,1024) if l1<l2]
# plot 7
k7=[]
for s in (1,2,4,8,16,32):
    k7+= [(s*KB,1,32,0,256*KB,8),(s*KB,1,32,4,256*KB,8),(s*KB,1,32,8,256*KB,8),(s*KB,1,32,16,256*KB,8),(s*KB,2,32,0,256*KB,8),(s*KB,4,32,0,256*KB,8)]
res=many(k1+k3+k4+k5+k7)
json.dump({str(k):v for k,v in res.items()},open('exp_results.json','w'))
print(len(res),'runs')
