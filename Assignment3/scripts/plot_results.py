from pathlib import Path
import csv
import matplotlib.pyplot as plt

base=Path(__file__).resolve().parent.parent
rows=list(csv.DictReader((base/'evidence/task05-replacement.csv').open()))
names=['FIFO','LRU','Optimal','Clock']
fig,axes=plt.subplots(1,2,figsize=(9,4.2),sharey=False)
for ax,work in zip(axes,['locality','random']):
    for alg in names:
        data=[r for r in rows if r['workload']==work and r['algorithm']==alg]
        ax.plot([int(r['frames']) for r in data],[int(r['faults']) for r in data],marker='o',label=alg)
    ax.set_title(f'{work.title()} reference string');ax.set_xlabel('Frames');ax.set_xticks(range(3,11));ax.grid(True,alpha=.3);ax.legend()
axes[0].set_ylabel('Page faults (100,000 references)')
fig.suptitle('Page-replacement faults by frame count');fig.tight_layout();fig.savefig(base/'plots/replacement_faults.png',dpi=170)
fig,ax=plt.subplots(figsize=(7.2,4.2))
for alg in names:
    data=[r for r in rows if r['workload']=='locality' and r['algorithm']==alg]
    ax.plot([int(r['frames']) for r in data],[int(r['faults']) for r in data],marker='o',label=alg)
ax.set_title('Working-set page faults versus frames');ax.set_xlabel('Frames');ax.set_ylabel('Faults');ax.set_xticks(range(3,11));ax.grid(True,alpha=.3);ax.legend();fig.tight_layout();fig.savefig(base/'plots/page_faults.png',dpi=170)
