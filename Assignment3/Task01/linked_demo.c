#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
extern const char *crypto_version(void);
extern int encrypt_demo(const char *);
int main(int argc,char **argv){printf("crypto library version=%s checksum=%d\n",crypto_version(),encrypt_demo("bank-record-42"));FILE*f=fopen("/proc/self/maps","r");if(f){char line[512];puts("program and library load addresses from /proc/self/maps:");while(fgets(line,sizeof line,f))if(strstr(line,"linked_demo")||strstr(line,"libbankcrypto"))fputs(line,stdout);fclose(f);}if(argc>1&&strcmp(argv[1],"--hold")==0){printf("pid=%ld holding mappings\n",(long)getpid());fflush(stdout);sleep(3);}return 0;}
