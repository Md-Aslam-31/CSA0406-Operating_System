#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CAP 16
#define TOTAL 10000
static int buf[CAP], in, out, producers, consumers, per_producer;
static int *seen; static sem_t empty_slots, full_slots; static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static long sum; static int errors;
static void *producer(void *arg){int id=*(int*)arg;for(int k=0;k<per_producer;k++){int item=id*per_producer+k;sem_wait(&empty_slots);pthread_mutex_lock(&mutex);buf[in]=item;in=(in+1)%CAP;pthread_mutex_unlock(&mutex);sem_post(&full_slots);}return NULL;}
static void *consumer(void *arg){(void)arg;for(;;){sem_wait(&full_slots);pthread_mutex_lock(&mutex);int item=buf[out];out=(out+1)%CAP;pthread_mutex_unlock(&mutex);sem_post(&empty_slots);if(item<0)break;if(item>=TOTAL){__sync_fetch_and_add(&errors,1);continue;}__sync_fetch_and_add(&seen[item],1);__sync_fetch_and_add(&sum,item);}return NULL;}
static int run(int np,int nc,int quiet){producers=np;consumers=nc;per_producer=TOTAL/np;if(per_producer*np!=TOTAL)return 1;memset(seen,0,TOTAL*sizeof *seen);in=out=0;sum=0;errors=0;sem_init(&empty_slots,0,CAP);sem_init(&full_slots,0,0);pthread_t p[4],c[4];int ids[4];for(int i=0;i<np;i++){ids[i]=i;pthread_create(&p[i],NULL,producer,&ids[i]);}for(int i=0;i<nc;i++)pthread_create(&c[i],NULL,consumer,NULL);for(int i=0;i<np;i++)pthread_join(p[i],NULL);for(int i=0;i<nc;i++){sem_wait(&empty_slots);pthread_mutex_lock(&mutex);buf[in]=-1;in=(in+1)%CAP;pthread_mutex_unlock(&mutex);sem_post(&full_slots);}for(int i=0;i<nc;i++)pthread_join(c[i],NULL);for(int i=0;i<TOTAL;i++)if(seen[i]!=1)errors++;long expected=(long)TOTAL*(TOTAL-1)/2;if(!quiet)printf("P=%d C=%d: consumed=%d sum=%ld expected=%ld duplicate/missing=%d\n",np,nc,TOTAL,sum,expected,errors);sem_destroy(&empty_slots);sem_destroy(&full_slots);return errors||sum!=expected;}
static volatile long counters[4][32];static pthread_mutex_t locks[4][32];
static long active_scope;
static void *bench(void *v){(void)v;const long loops=200000;for(long i=0;i<loops;i++){int train=(int)(i%4),coach=(int)((i/4)%8),idx=active_scope==2?train*8+coach:active_scope==1?train:0;pthread_mutex_lock(&locks[active_scope][idx]);counters[active_scope][idx]++;pthread_mutex_unlock(&locks[active_scope][idx]);}return NULL;}
int main(void){seen=calloc(TOTAL,sizeof *seen);if(!seen)return 1;if(run(1,1,0)||run(4,4,0))return 1;for(int i=0;i<100;i++)if(run(4,4,1))return 1;puts("100 repeated P=4/C=4 runs: PASS (each item exactly once)");for(int i=0;i<4;i++)for(int j=0;j<32;j++)pthread_mutex_init(&locks[i][j],NULL);pthread_t t[4];struct timespec a,b;puts("lock_scope,seconds,operations_per_second");const char*names[]={"global","per_train","per_coach"};for(int scope=0;scope<3;scope++){active_scope=scope;clock_gettime(CLOCK_MONOTONIC,&a);for(long i=0;i<4;i++)pthread_create(&t[i],NULL,bench,(void*)i);for(int i=0;i<4;i++)pthread_join(t[i],NULL);clock_gettime(CLOCK_MONOTONIC,&b);double s=b.tv_sec-a.tv_sec+(b.tv_nsec-a.tv_nsec)/1e9;printf("%s,%.6f,%.0f\n",names[scope],s,800000.0/s);}free(seen);return 0;}
