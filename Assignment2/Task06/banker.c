#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define P 5
#define R 3
static int avail[R]={3,3,2};
static int maxm[P][R]={{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
static int alloc[P][R]={{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
static int is_safe(int seq[P]){int work[R],finish[P]={0},count=0;memcpy(work,avail,sizeof work);while(count<P){int found=0;for(int p=0;p<P;p++){if(finish[p])continue;int ok=1;for(int r=0;r<R;r++)if(maxm[p][r]-alloc[p][r]>work[r])ok=0;if(ok){for(int r=0;r<R;r++)work[r]+=alloc[p][r];finish[p]=1;seq[count++]=p;found=1;}}if(!found)return 0;}return 1;}
static void request(int p,const int req[R]){for(int r=0;r<R;r++)if(req[r]>maxm[p][r]-alloc[p][r]||req[r]>avail[r]){printf("P%d request: must wait (not available)\n",p);return;}for(int r=0;r<R;r++)avail[r]-=req[r],alloc[p][r]+=req[r];int seq[P];if(is_safe(seq)){printf("P%d request: granted, safe sequence",p);for(int i=0;i<P;i++)printf(" P%d",seq[i]);puts("");}else{for(int r=0;r<R;r++)avail[r]+=req[r],alloc[p][r]-=req[r];printf("P%d request: denied (unsafe)\n",p);}}
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static double benchmark(int n){int *allocation=calloc((size_t)n,sizeof *allocation),*finished=calloc((size_t)n,sizeof *finished);if(!allocation||!finished)exit(2);for(int p=0;p<n;p++)allocation[p]=1;int rounds=1000000/(n*n);if(rounds<20)rounds=20;double s=now();volatile int sink=0;for(int z=0;z<rounds;z++){memset(finished,0,(size_t)n*sizeof *finished);int work=1,count=0;while(count<n){int found=0;for(int p=0;p<n;p++){int need=n-p-1;if(!finished[p]&&need<=work){work+=allocation[p];finished[p]=1;count++;found=1;}}if(!found)break;}sink+=count;}double t=(now()-s)/rounds;(void)sink;free(allocation);free(finished);return t;}
int main(void){int seq[P];if(is_safe(seq)){printf("initial state safe:");for(int i=0;i<P;i++)printf(" P%d",seq[i]);puts("");}request(1,(int[R]){1,0,2});request(4,(int[R]){3,3,0});request(0,(int[R]){0,2,0});puts("n_processes,mean_safety_scan_seconds");int ns[]={10,100,1000};for(int i=0;i<3;i++)printf("%d,%.9f\n",ns[i],benchmark(ns[i]));return 0;}
