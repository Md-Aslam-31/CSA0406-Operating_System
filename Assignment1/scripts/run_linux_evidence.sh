#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p evidence
printf 'OS: '; . /etc/os-release; printf '%s %s\n' "$NAME" "$VERSION_ID"
printf 'Compiler: '; gcc --version | head -n 1
for task in Task06 Task07; do gcc -std=c11 -Wall -Wextra -pthread "$task"/*.c -o "$task/${task,,}"; done
(cd Task06 && ./task06) 2>&1 | tee evidence/task06-output.txt
(cd Task07 && ./task07) 2>&1 | tee evidence/task07-output.txt
strace -c -e trace=read,write,openat,close ./Task06/task06 2> evidence/task06-strace-summary.txt
/usr/bin/time -v ./Task06/task06 > evidence/task06-time-output.txt 2> evidence/task06-time.txt
# Launch 1,000 short-lived child processes, then 1,000 joinable pthreads.
gcc -std=c11 -Wall -Wextra -pthread -x c -o /tmp/assignment1_thread_bench - <<'C'
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static void *noop(void *p) { return p; }
static double seconds(struct timespec a, struct timespec b) { return (b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9; }
int main(void) {
 struct timespec a,b; clock_gettime(CLOCK_MONOTONIC,&a);
 for(int i=0;i<1000;i++){ pid_t p=fork(); if(p==0)_exit(0); if(p<0)return 2; if(waitpid(p,0,0)<0)return 3; }
 clock_gettime(CLOCK_MONOTONIC,&b); printf("1000 fork+wait sequential seconds: %.6f\n",seconds(a,b));
 pthread_t t[1000]; clock_gettime(CLOCK_MONOTONIC,&a);
 for(int i=0;i<1000;i++)if(pthread_create(&t[i],0,noop,0))return 4;
 for(int i=0;i<1000;i++)if(pthread_join(t[i],0))return 5;
 clock_gettime(CLOCK_MONOTONIC,&b); printf("1000 pthread create+join seconds: %.6f\n",seconds(a,b));
}
C
/usr/bin/time -v /tmp/assignment1_thread_bench > evidence/task10-process-thread.txt 2> evidence/task10-time.txt
