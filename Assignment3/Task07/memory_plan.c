#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#define HOT (16u*1024u)
#define TRACE 20000
static double ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1e9+t.tv_nsec;}
static int simulate(int pin){int frames[128],last[128],count=0,faults=0,rtfault=0;for(int i=0;i<TRACE;i++){int slot=i%1000,page,control=(slot==0);if(control)page=(i/1000)%16;else if(slot<990)page=100+(int)((i*1103515245u+12345u)%256);else page=400+(i%24);int k=-1;for(int j=0;j<count;j++)if(frames[j]==page)k=j;if(k<0){faults++;if(control&&i>=16000)rtfault++;if(count<128){k=count++;frames[k]=page;}else{int v=-1;for(int j=0;j<128;j++){if(pin&&frames[j]>=0&&frames[j]<16)continue;if(v<0||last[j]<last[v])v=j;}if(v<0)return -1;k=v;frames[k]=page;}}last[k]=i;}printf("pin_control=%d simulation_faults=%d control_faults_after_warmup=%d\n",pin,faults,rtfault);return rtfault;}
static int cmp(const void*a,const void*b){double x=*(const double*)a,y=*(const double*)b;return (x>y)-(x<y);}
static void timing(int locked){size_t ps=(size_t)sysconf(_SC_PAGESIZE),bytes=HOT;volatile unsigned char*p=calloc(1,bytes);if(!p){perror("calloc");return;}if(locked&&mlock((const void*)p,bytes)){printf("mlock failed: %s\n",strerror(errno));free((void*)p);return;}for(size_t i=0;i<bytes;i+=ps)p[i]=1;double samples[5000],worst=0;for(int k=0;k<5000;k++){double a=ns();for(size_t i=0;i<bytes;i+=ps)p[i]++;double d=ns()-a;samples[k]=d;if(d>worst)worst=d;}qsort(samples,5000,sizeof samples[0],cmp);printf("memory_lock=%d,working_set=%zu,p50_ns=%.0f,p99_ns=%.0f,max_ns=%.0f\n",locked,bytes,samples[2500],samples[4950],worst);if(locked)munlock((const void*)p,bytes);free((void*)p);}
int main(void){long ps=sysconf(_SC_PAGESIZE);struct rlimit lim;if(getrlimit(RLIMIT_MEMLOCK,&lim)==0)printf("page_size=%ld,memlock_limit=%llu bytes\n",ps,(unsigned long long)lim.rlim_cur);puts("memory budget MiB: control 128 (locked reserve), ML 1536, logging 128, OS/services 256 = 2048; no swap");puts("time-critical requirement: no page faults in control loop; prefault and mlock the fixed 128 MiB working set, isolate CPU, measure deadline on target hardware");timing(0);timing(1);puts("task working-set page replacement:");int a=simulate(0),b=simulate(1);printf("control faults without pin=%d, with pin=%d\n",a,b);puts("4th 500MiB task: over 2GiB budget by 500MiB; cap/reduce ML model or move task off device, never reclaim RT reserve");return b==0?0:1;}
