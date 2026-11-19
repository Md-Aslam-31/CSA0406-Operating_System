#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
static unsigned rnd(unsigned *s){*s=*s*1664525u+1013904223u;return *s;}
static void eat(double h){double tlb=20,mem=100;printf("TLB hit %.0f%% EAT %.2f ns\n",h*100,h*(tlb+mem)+(1-h)*(tlb+2*mem));}
int main(void){printf("page translation TODO is built separately; system page size=%ld bytes\n",sysconf(_SC_PAGESIZE));printf("32-bit, 4KiB pages: 2^20 entries x 4 B = 4 MiB (flat table)\n48-bit, 4KiB pages: 2^36 entries x 8 B = 512 GiB (flat table; canonical user range may be smaller)\n");eat(.80);eat(.95);eat(.99);const unsigned ps[]={1024,4096,2097152};unsigned seed=192572147u;for(int j=0;j<3;j++){unsigned long long waste=0;for(int i=0;i<100;i++){size_t size=1+(rnd(&seed)%50000000u);waste+=(ps[j]-(size%ps[j]))%ps[j];}printf("100 deterministic process sizes, page=%u B: total internal waste=%llu B, mean=%.1f B\n",ps[j],waste,(double)waste/100);}int a[]={0,4095,4096,32767,32768};puts("boundary checks (page table has 8 entries, 4 KiB pages):");for(int i=0;i<5;i++){unsigned pg=(unsigned)a[i]/4096,off=(unsigned)a[i]%4096;printf("address %d: page %u offset %u -> %s\n",a[i],pg,off,pg>=8?"invalid":"within table range");}return 0;}
