#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAXF 16
#define LEN 100000
static unsigned state=192572147u;
static unsigned rnd(void){state=state*1664525u+1013904223u;return state;}
static int find(const int*f,int n,int x){for(int i=0;i<n;i++)if(f[i]==x)return i;return -1;}
static int fifo(const int*r,int n,int cap){int f[MAXF],used=0,next=0,miss=0;for(int i=0;i<n;i++){if(find(f,used,r[i])>=0)continue;miss++;if(used<cap)f[used++]=r[i];else{f[next]=r[i];next=(next+1)%cap;}}return miss;}
static int lru(const int*r,int n,int cap){int f[MAXF],last[MAXF],used=0,miss=0;for(int i=0;i<n;i++){int k=find(f,used,r[i]);if(k>=0){last[k]=i;continue;}miss++;if(used<cap){k=used++;f[k]=r[i];}else{k=0;for(int j=1;j<cap;j++)if(last[j]<last[k])k=j;f[k]=r[i];}last[k]=i;}return miss;}
static int optimal(const int*r,int n,int cap){int f[MAXF],nextuse[MAXF],used=0,miss=0,*after=malloc((size_t)n*sizeof *after);int last[4096];if(!after)return -1;for(int p=0;p<4096;p++)last[p]=n;for(int i=n-1;i>=0;i--){after[i]=last[r[i]];last[r[i]]=i;}for(int i=0;i<n;i++){int k=find(f,used,r[i]);if(k>=0){nextuse[k]=after[i];continue;}miss++;if(used<cap)k=used++,f[k]=r[i];else{k=0;for(int j=1;j<cap;j++)if(nextuse[j]>nextuse[k])k=j;f[k]=r[i];}nextuse[k]=after[i];}free(after);return miss;}
static int clock_alg(const int*r,int n,int cap){int f[MAXF],bit[MAXF]={0},used=0,hand=0,miss=0;for(int i=0;i<n;i++){int k=find(f,used,r[i]);if(k>=0){bit[k]=1;continue;}miss++;if(used<cap){f[used]=r[i];bit[used++]=1;continue;}while(bit[hand]){bit[hand]=0;hand=(hand+1)%cap;}f[hand]=r[i];bit[hand]=1;hand=(hand+1)%cap;}return miss;}
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
int main(void){int*loc=malloc(LEN*sizeof *loc),*random=malloc(LEN*sizeof *random);if(!loc||!random)return 2;unsigned base=0;for(int i=0;i<LEN;i++){if(i%1000==0)base=rnd()%4096;loc[i]=(int)((base+(rnd()%8))%4096);random[i]=(int)(rnd()%4096);}int(*fn[])(const int*,int,int)={fifo,lru,optimal,clock_alg};const char*names[]={"FIFO","LRU","Optimal","Clock"};puts("workload,algorithm,frames,faults,seconds");for(int w=0;w<2;w++){const int*r=w?random:loc;const char*wn=w?"random":"locality";for(int cap=3;cap<=10;cap++)for(int a=0;a<4;a++){double t=now();int faults=fn[a](r,LEN,cap);double elapsed=now()-t;printf("%s,%s,%d,%d,%.6f\n",wn,names[a],cap,faults,elapsed);}}int sample[]={1,2,3,4,1,2,5,1,2,3,4,5};printf("hand-check Belady FIFO faults 3=%d 4=%d\n",fifo(sample,12,3),fifo(sample,12,4));free(loc);free(random);return 0;}
