#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static int pipe_test(size_t n){int fd[2];if(pipe(fd))return 1;pid_t p=fork();if(p<0)return 1;if(!p){close(fd[1]);char b[65536];size_t got=0;ssize_t r;while((r=read(fd[0],b,sizeof b))>0)got+=(size_t)r;close(fd[0]);_exit(got==n?0:2);}close(fd[0]);char *b=calloc(1,65536);if(!b)return 1;size_t sent=0;while(sent<n){size_t z=n-sent<65536?n-sent:65536;ssize_t w=write(fd[1],b,z);if(w<0){if(errno==EINTR)continue;perror("write");return 1;}sent+=(size_t)w;}free(b);close(fd[1]);int st;waitpid(p,&st,0);return !WIFEXITED(st)||WEXITSTATUS(st);}
static int shm_test(size_t n){char name[80];snprintf(name,sizeof name,"/osipc_%ld",(long)getpid());int fd=shm_open(name,O_CREAT|O_EXCL|O_RDWR,0600);if(fd<0)return 1;if(ftruncate(fd,(off_t)n)){perror("ftruncate");return 1;}char *p=mmap(NULL,n,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);if(p==MAP_FAILED)return 1;int ready[2];if(pipe(ready))return 1;pid_t c=fork();if(c<0)return 1;if(!c){close(ready[1]);char token;if(read(ready[0],&token,1)!=1)_exit(2);volatile unsigned char sum=0;for(size_t i=0;i<n;i+=4096)sum+=(unsigned char)p[i];(void)sum;_exit(0);}close(ready[0]);memset(p,0,n);char token='x';if(write(ready[1],&token,1)!=1)return 1;close(ready[1]);int st;waitpid(c,&st,0);munmap(p,n);close(fd);shm_unlink(name);return !WIFEXITED(st)||WEXITSTATUS(st);}
int main(void){size_t mb[]={1,10,100};puts("method,MB,seconds,MiB_per_second");for(int i=0;i<3;i++){size_t n=mb[i]*1024u*1024u;double s=now();int e=pipe_test(n);double t=now()-s;if(e)return 1;printf("pipe,%zu,%.6f,%.2f\n",mb[i],t,mb[i]/t);s=now();e=shm_test(n);t=now()-s;if(e)return 1;printf("shared_memory,%zu,%.6f,%.2f\n",mb[i],t,mb[i]/t);}return 0;}
