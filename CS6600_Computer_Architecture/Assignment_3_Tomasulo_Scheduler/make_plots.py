import json,math
import matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
r=json.load(open('ooo_sweep.json'))
plt.rcParams.update({'font.size':10,'axes.grid':True,'grid.alpha':.3})
for tr,name in (('gcc','GCC'),('perl','PERL')):
    fig,ax=plt.subplots(figsize=(5.6,3.8))
    for N,mk in zip((2,4,8),'osd'):
        ax.plot([math.log2(S) for S in (8,16,32,64,128,256)],[r[f'{tr},{N},{S}']['ipc'] for S in (8,16,32,64,128,256)],marker=mk,label=f'N = {N}')
    for N in (2,4,8): ax.axhline(N,color='gray',linestyle=':',linewidth=.7)
    ax.set_xlabel('log$_2$ (scheduling queue size S)'); ax.set_ylabel('IPC'); ax.set_title(f'{name} trace: IPC vs. S (dotted: IPC = N)'); ax.legend(fontsize=8)
    fig.tight_layout(); fig.savefig(f'{name}_Benchmark.png',dpi=160); plt.close(fig)
