#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void touch(size_t bytes,int random_order,const char*label){size_t page=(size_t)sysconf(_SC_PAGESIZE),n=(bytes+page-1)/page;volatile unsigned char*p=calloc(n,page);if(!p){perror("calloc");return;}unsigned long seed=192572147;struct rusage a,b;getrusage(RUSAGE_SELF,&a);double t=now();for(size_t i=0;i<n;i++){size_t k=i;if(random_order){seed=seed*1664525+1013904223;k=(seed% n);}p[k*page]=(unsigned char)i;}double elapsed=now()-t;getrusage(RUSAGE_SELF,&b);printf("%s,%zu,%.6f,%ld,%ld,checksum=%u\n",label,bytes,elapsed,b.ru_minflt-a.ru_minflt,b.ru_majflt-a.ru_majflt,p[(n-1)*page]);free((void*)p);}
static void limited_profile(size_t mib){pid_t c=fork();if(c==0){struct rlimit lim={512u*1024*1024,512u*1024*1024};setrlimit(RLIMIT_AS,&lim);size_t bytes=mib*1024u*1024u,page=(size_t)sysconf(_SC_PAGESIZE);volatile unsigned char*p=malloc(bytes);if(!p){printf("limit=512MiB request=%zuMiB allocation=rejected (%s)\n",mib,strerror(errno));fflush(stdout);_exit(0);}struct rusage a,b;getrusage(RUSAGE_SELF,&a);double t=now();for(size_t i=0;i<bytes;i+=page)p[i]=1;double elapsed=now()-t;getrusage(RUSAGE_SELF,&b);printf("limit=512MiB touched=%zuMiB seconds=%.6f minor_faults=%ld major_faults=%ld\n",mib,elapsed,b.ru_minflt-a.ru_minflt,b.ru_majflt-a.ru_majflt);fflush(stdout);_exit(0);}int st;waitpid(c,&st,0);}
static void smaps(const char*label){FILE*f=fopen("/proc/self/smaps_rollup","r");if(!f)return;char line[256];printf("%s ",label);while(fgets(line,sizeof line,f))if(!strncmp(line,"Rss:",4)||!strncmp(line,"Pss:",4)||!strncmp(line,"Private_Dirty:",14))printf("%s",line);fclose(f);fflush(stdout);}
int main(void){size_t p=(size_t)sysconf(_SC_PAGESIZE);printf("page size=%zu; EAT with memory=100 ns and fault service=8 ms\n",p);double rates[]={1e-3,1e-5,1e-6};for(int i=0;i<3;i++)printf("fault rate %.8f EAT %.3f ns\n",rates[i],(1-rates[i])*100+rates[i]*(8000000+100));printf("10%% slowdown maximum page-fault rate=%.9f (about 1 per 800000 references)\n",10.0/7999900.0);puts("pattern,bytes,seconds,minor_fault_delta,major_fault_delta,checksum");touch(64u*1024*1024,0,"sequential");touch(64u*1024*1024,1,"random");size_t bytes=64u*1024*1024;unsigned char*mem=malloc(bytes);if(mem){memset(mem,1,bytes);smaps("parent before fork:");int ready[2];pipe(ready);pid_t c=fork();if(c==0){close(ready[0]);smaps("child before write:");for(size_t i=0;i<bytes;i+=p)mem[i]++;smaps("child after COW writes:");write(ready[1],"x",1);_exit(0);}close(ready[1]);char token;read(ready[0],&token,1);waitpid(c,NULL,0);smaps("parent after child writes:");free(mem);}size_t profile[]={64,128,256,384,768};for(size_t i=0;i<5;i++)limited_profile(profile[i]);printf("working-set reference simulation: see Q5 page-fault-vs-frames plot (FIFO/LRU/Optimal/Clock)\n");return 0;}
