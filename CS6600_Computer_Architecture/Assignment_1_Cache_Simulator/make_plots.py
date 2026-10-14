import json,ast,math
import numpy as np
import matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
R={ast.literal_eval(k):v for k,v in json.load(open('exp_results.json')).items()}
KB=1024
plt.rcParams.update({'font.size':10,'axes.grid':True,'grid.alpha':.3})
def val(k,key): 
    v=R[k]; return v[key] if v['valid'] or key in ('mr',) else float('nan')
NAMES=[('Direct-mapped',lambda s:1),('2-way',lambda s:2),('4-way',lambda s:4),('8-way',lambda s:8),('Fully associative',lambda s:s//32)]
def curves(path,sizes,key,l2,ylabel,title,logy=False):
    fig,ax=plt.subplots(figsize=(6.2,4.2))
    for (n,af),mk in zip(NAMES,'osd^v'):
        xs=[math.log2(s) for s in sizes]; ys=[val((s,af(s),32,0,l2,8 if l2 else 0),key) for s in sizes]
        ax.plot(xs,ys,marker=mk,label=n,linewidth=1.4,markersize=4)
    ax.set_xlabel('log$_2$(L1 size in bytes)'); ax.set_ylabel(ylabel); ax.set_title(title)
    if logy: ax.set_yscale('log')
    ax.legend(fontsize=8); fig.tight_layout(); fig.savefig(path); plt.close(fig)
s1=[2**e for e in range(11,21)]
curves('plots/plot1_missrate.pdf',s1,'mr',0,'L1 miss rate','Plot 1: L1 miss rate vs. size (block 32 B, no L2)',logy=True)
curves('plots/plot2_aat.pdf',s1,'aat',0,'AAT (ns)','Plot 2: AAT vs. L1 size (block 32 B, no L2)')
curves('plots/plot3_aat_l2.pdf',[2**e for e in range(11,18)],'aat',256*KB,'AAT (ns)','Plot 3: AAT vs. L1 size with a 256 KB 8-way L2')
# plot 4
fig,ax=plt.subplots(figsize=(6.2,4.2))
for s in (1,2,4,8,16,32):
    ax.plot([4,5,6,7],[R[(s*KB,4,bs,0,0,0)]['mr'] for bs in (16,32,64,128)],marker='o',markersize=4,label=f'{s} KB')
ax.set_xlabel('log$_2$(block size in bytes)'); ax.set_ylabel('L1 miss rate'); ax.set_title('Plot 4: L1 miss rate vs. block size (4-way, no L2)'); ax.legend(fontsize=8,ncol=2)
fig.tight_layout(); fig.savefig('plots/plot4_blocksize.pdf'); plt.close(fig)
# plots 5/6
def surf(path,key,title,zlabel,scale=1.0):
    L1=[1,2,4,8,16,32,64]; L2=[32,64,128,256,512,1024]
    pts=[(math.log2(l1*KB),math.log2(l2*KB),R[(l1*KB,4,32,0,l2*KB,8)][key]/scale) for l1 in L1 for l2 in L2 if l1<l2 and R[(l1*KB,4,32,0,l2*KB,8)]['valid']]
    fig=plt.figure(figsize=(6.4,4.8)); ax=fig.add_subplot(111,projection='3d')
    x,y,z=map(np.array,zip(*pts))
    ax.plot_trisurf(x,y,z,cmap='viridis',alpha=.85,edgecolor='k',linewidth=.3); ax.scatter(x,y,z,color='r',s=10)
    ax.set_xlabel('log$_2$(L1 size)'); ax.set_ylabel('log$_2$(L2 size)'); ax.set_zlabel(zlabel); ax.set_title(title,fontsize=10); ax.view_init(28,-125)
    fig.tight_layout(); fig.savefig(path); plt.close(fig)
surf('plots/plot5_aat_surface.pdf','aat','Plot 5: AAT vs. L1 and L2 size (4-way L1, 8-way L2, block 32 B)','AAT (ns)')
surf('plots/plot6_edp_surface.pdf','edp','Plot 6: EDP vs. L1 and L2 size','EDP ($10^6$ nJ$\\cdot$ns)',1e6)
# plot 7
fig,ax=plt.subplots(figsize=(6.2,4.2))
cfgs=[('DM',1,0),('DM + 4-entry VC',1,4),('DM + 8-entry VC',1,8),('DM + 16-entry VC',1,16),('2-way',2,0),('4-way',4,0)]
for (n,a,vc),mk in zip(cfgs,'osd^vx'):
    xs=[math.log2(s*KB) for s in (1,2,4,8,16,32)]; ys=[val((s*KB,a,32,vc,256*KB,8),'aat') for s in (1,2,4,8,16,32)]
    ax.plot(xs,ys,marker=mk,label=n,linewidth=1.4,markersize=4)
ax.set_xlabel('log$_2$(L1 size in bytes)'); ax.set_ylabel('AAT (ns)'); ax.set_title('Plot 7: victim cache vs. associativity (256 KB 8-way L2)'); ax.legend(fontsize=8)
fig.tight_layout(); fig.savefig('plots/plot7_victim.pdf'); plt.close(fig)
print('ok')
