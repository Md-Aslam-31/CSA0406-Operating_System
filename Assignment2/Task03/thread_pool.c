#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FRAMES 64
#define WORK 1500000u
static uint64_t result[FRAMES];
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
typedef struct{int id,stride;} Arg;
static void *process(void *v){Arg *a=v;for(int f=a->id;f<FRAMES;f+=a->stride){uint64_t x=(uint64_t)f+1;for(unsigned i=0;i<WORK;i++)x=(x*1664525u+1013904223u)^(x>>13);result[f]=x;}return NULL;}
static double run_pool(int n){pthread_t th[FRAMES];Arg a[FRAMES];double start=now();for(int i=0;i<n;i++){a[i]=(Arg){i,n};if(pthread_create(&th[i],NULL,process,&a[i])){perror("pthread_create");exit(1);}}for(int i=0;i<n;i++)pthread_join(th[i],NULL);return now()-start;}
int main(void){double serial=run_pool(1);printf("frames=%d work_per_frame=%u checksum=%llu\n",FRAMES,WORK,(unsigned long long)result[0]);printf("threads,seconds,speedup\n");int sizes[]={1,4,8,16,64};for(int i=0;i<5;i++){double t=run_pool(sizes[i]);printf("%d,%.6f,%.3f\n",sizes[i],t,serial/t);}return 0;}
