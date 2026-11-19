#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
typedef struct{const char*name;unsigned long base,limit;} Segment;
int main(void){Segment s[]={{"code",0x10000,0x4000},{"data",0x20000,0x8000},{"heap",0x30000,0x10000},{"stack",0x70000,0x8000}};struct{int seg;unsigned long off;} a[]={{0,0},{0,16383},{1,4100},{2,8192},{3,32},{1,32767},{2,65535},{3,32768}};puts("segment,offset,limit,result");for(int i=0;i<8;i++){int n=a[i].seg;printf("%s,%lu,%lu,",s[n].name,a[i].off,s[n].limit);if(a[i].off>=s[n].limit)puts("protection fault (offset >= limit)");else printf("physical-linear 0x%lx\n",s[n].base+a[i].off);}puts("segmentation + paging: logical (segment,offset) -> segment base+offset -> page number/offset -> frame+offset -> physical address");FILE*f=fopen("/proc/self/maps","r");if(f){char line[512];puts("this process /proc maps:");while(fgets(line,sizeof line,f))fputs(line,stdout);fclose(f);}pid_t c=fork();if(c==0){volatile char*p=mmap(NULL,4096,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);if(p==MAP_FAILED)_exit(2);*p=1;_exit(0);}int st;waitpid(c,&st,0);if(WIFSIGNALED(st))printf("protected-page access terminated child with signal %d (%s)\n",WTERMSIG(st),strsignal(WTERMSIG(st)));else printf("protection test child exit=%d\n",WEXITSTATUS(st));unsigned seed=71;size_t sizes[20],used=0,arena=64u*1024u*1024u;for(int i=0;i<20;i++){seed=seed*1103515245u+12345u;sizes[i]=65536+(seed%2000000);used+=sizes[i];}size_t freed=0,largest=arena-used;for(int i=0;i<20;i+=2){freed+=sizes[i];if(sizes[i]>largest)largest=sizes[i];}printf("20 segment simulation: arena=%zu allocated=%zu freed=%zu total_free=%zu largest_hole=%zu external_fragmentation=%zu\n",arena,used,freed,arena-used+freed,largest,arena-used+freed-largest);return 0;}
