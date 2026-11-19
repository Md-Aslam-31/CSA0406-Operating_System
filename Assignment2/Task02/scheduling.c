#include <stdio.h>
#include <string.h>

#define N 4
typedef struct { const char *name; int at, bt, pr; } Job;
static const Job jobs[N] = {{"P1",0,8,2},{"P2",1,4,1},{"P3",2,9,4},{"P4",3,5,3}};
static void report(const char *label, int finish[N], double response[N], int switches) {
    double w=0,t=0,r=0;
    printf("%s\nProcess  CT  TAT  WT  RT\n",label);
    for(int i=0;i<N;i++){int tat=finish[i]-jobs[i].at, wt=tat-jobs[i].bt; w+=wt;t+=tat;r+=response[i];printf("%-7s %2d %4d %3d %3.0f\n",jobs[i].name,finish[i],tat,wt,response[i]);}
    printf("avg TAT %.2f WT %.2f RT %.2f; context switches %d\n",t/N,w/N,r/N,switches);
}
static void nonpreemptive(int mode,const char *label){
    int done[N]={0},f[N]={0},time=0,sw=0;double rt[N]={0};
    for(int k=0;k<N;k++){int pick=-1;for(int i=0;i<N;i++)if(!done[i]&&jobs[i].at<=time&&(pick<0||(mode==0?jobs[i].at<jobs[pick].at:mode==1?jobs[i].bt<jobs[pick].bt:jobs[i].pr<jobs[pick].pr)))pick=i;
        if(pick<0){time++;k--;continue;}rt[pick]=time-jobs[pick].at; if(k)sw++;printf("%s(%d-%d) ",jobs[pick].name,time,time+jobs[pick].bt);time+=jobs[pick].bt;f[pick]=time;done[pick]=1;}
    puts("");report(label,f,rt,sw);
}
static void srtf(void){int left[N],f[N]={0},first[N],time=0,last=-1,segment_start=0,sw=0,done=0;for(int i=0;i<N;i++)left[i]=jobs[i].bt,first[i]=-1;printf("SRTF Gantt: ");
    while(done<N){int p=-1;for(int i=0;i<N;i++)if(left[i]>0&&jobs[i].at<=time&&(p<0||left[i]<left[p]))p=i;if(p<0){time++;continue;}if(p!=last){if(last>=0){printf("%s(%d-%d) ",jobs[last].name,segment_start,time);sw++;}segment_start=time;}if(first[p]<0)first[p]=time;left[p]--;time++;if(!left[p])f[p]=time,done++;last=p;}
    if(last>=0)printf("%s(%d-%d)\n",jobs[last].name,segment_start,time);double rt[N];for(int i=0;i<N;i++)rt[i]=first[i]-jobs[i].at;report("SRTF",f,rt,sw);
}
static void rr(int q){int left[N],f[N]={0},first[N],qbuf[256],head=0,tail=0,time=0,done=0,sw=0,last=-1;for(int i=0;i<N;i++)left[i]=jobs[i].bt,first[i]=-1;int added[N]={0};for(int i=0;i<N;i++)if(jobs[i].at==0)qbuf[tail++]=i,added[i]=1;printf("RR q=%d Gantt: ",q);
    while(done<N){if(head==tail){time++;for(int i=0;i<N;i++)if(!added[i]&&jobs[i].at<=time)qbuf[tail++]=i,added[i]=1;continue;}int p=qbuf[head++];if(last>=0&&last!=p)sw++;last=p;if(first[p]<0)first[p]=time;int d=left[p]<q?left[p]:q;printf("%s(%d-%d) ",jobs[p].name,time,time+d);time+=d;left[p]-=d;for(int i=0;i<N;i++)if(!added[i]&&jobs[i].at<=time)qbuf[tail++]=i,added[i]=1;if(left[p])qbuf[tail++]=p;else f[p]=time,done++;}
    puts("");double rt[N];for(int i=0;i<N;i++)rt[i]=first[i]-jobs[i].at;char label[32];snprintf(label,sizeof label,"RR quantum %d",q);report(label,f,rt,sw);
}
int main(void){puts("FCFS Gantt:");nonpreemptive(0,"FCFS");puts("SJF Gantt:");nonpreemptive(1,"SJF (non-preemptive)");srtf();puts("Priority Gantt (lower number = higher priority):");nonpreemptive(2,"Priority (non-preemptive)");rr(1);rr(3);rr(5);rr(20);return 0;}
