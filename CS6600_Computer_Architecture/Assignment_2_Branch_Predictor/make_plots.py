import json
import matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
r=json.load(open('bp_sweep.json'))
plt.rcParams.update({'font.size':10,'axes.grid':True,'grid.alpha':.3})
import os; os.makedirs('SimulationGraphs',exist_ok=True)
def bimodal(tr,title):
    fig,ax=plt.subplots(figsize=(5.6,3.8))
    ms=list(range(7,17)); ys=[r[f'bimodal,{tr},{m}'] for m in ms]
    ax.plot(ms[:6],ys[:6],'o-',label='7 $\\leq$ m $\\leq$ 12'); ax.plot(ms[5:],ys[5:],'o--',color='gray',label='m > 12 (up to 16 KB budget)')
    ax.set_xlabel('m (log$_2$ of table entries)'); ax.set_ylabel('misprediction rate (%)'); ax.set_title(title); ax.legend(fontsize=8)
    fig.tight_layout(); fig.savefig(f'SimulationGraphs/Bimodal_{tr}_benchmark.png',dpi=160); plt.close(fig)
bimodal('gcc','Bimodal predictor, gcc trace'); bimodal('jpeg','Bimodal predictor, jpeg trace')
def gshare(tr,title,mmax,name):
    fig,ax=plt.subplots(figsize=(5.6,3.8))
    for n in range(2,mmax+1,2):
        ms=list(range(max(7,n),mmax+1))
        if not ms: continue
        ax.plot(ms,[r[f'gshare,{tr},{m},{n}'] for m in ms],marker='o',markersize=3.5,label=f'n = {n}')
    ax.set_xlabel('m (log$_2$ of table entries)'); ax.set_ylabel('misprediction rate (%)'); ax.set_title(title); ax.legend(fontsize=8,ncol=2)
    fig.tight_layout(); fig.savefig(f'SimulationGraphs/{name}.png',dpi=160); plt.close(fig)
gshare('gcc','gshare predictor, gcc trace (7 $\\leq$ m $\\leq$ 12)',12,'Gshare_gcc_benchmark'); gshare('jpeg','gshare predictor, jpeg trace (7 $\\leq$ m $\\leq$ 12)',12,'Gshare_jpeg_benchmark')
gshare('gcc','gshare, gcc trace, extended to the 16 KB budget (m $\\leq$ 16)',16,'Gshare_gcc_extended'); gshare('jpeg','gshare, jpeg trace, extended to the 16 KB budget (m $\\leq$ 16)',16,'Gshare_jpeg_extended')
print('ok')
