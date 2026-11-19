#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOOPS 500000
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
static atomic_long atomic_count;
static atomic_flag spin=ATOMIC_FLAG_INIT;
static long mutex_count,spin_count;
static int mode;
static double seconds(clockid_t id){struct timespec t;clock_gettime(id,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void *worker(void *unused){(void)unused;for(int i=0;i<LOOPS;i++){if(mode==0){pthread_mutex_lock(&mutex);mutex_count++;pthread_mutex_unlock(&mutex);}else if(mode==1){atomic_fetch_add_explicit(&atomic_count,1,memory_order_relaxed);}else{while(atomic_flag_test_and_set_explicit(&spin,memory_order_acquire)){}spin_count++;atomic_flag_clear_explicit(&spin,memory_order_release);}}return NULL;}
int main(void){const char *names[]={"mutex","atomic_fetch_add","spin_lock"};int counts[]={1,4,16};puts("primitive,threads,wall_s,process_cpu_s,ns_per_update,cpu_to_wall");for(mode=0;mode<3;mode++)for(int c=0;c<3;c++){int n=counts[c];pthread_t t[16];mutex_count=spin_count=0;atomic_store(&atomic_count,0);atomic_flag_clear(&spin);double w0=seconds(CLOCK_MONOTONIC),c0=seconds(CLOCK_PROCESS_CPUTIME_ID);for(int i=0;i<n;i++)if(pthread_create(&t[i],NULL,worker,NULL)){perror("pthread_create");return 1;}for(int i=0;i<n;i++)pthread_join(t[i],NULL);double cpu=seconds(CLOCK_PROCESS_CPUTIME_ID)-c0,wall=seconds(CLOCK_MONOTONIC)-w0;long value=mode==0?mutex_count:mode==1?atomic_load(&atomic_count):spin_count;long expected=(long)n*LOOPS;if(value!=expected){fprintf(stderr,"%s count mismatch: %ld != %ld\n",names[mode],value,expected);return 1;}printf("%s,%d,%.6f,%.6f,%.2f,%.2f\n",names[mode],n,wall,cpu,wall*1e9/expected,cpu/wall);}pthread_mutex_destroy(&mutex);return 0;}
