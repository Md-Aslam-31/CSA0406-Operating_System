#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdio.h>
typedef const char *(*version_fn)(void); typedef int (*encrypt_fn)(const char *);
int main(void){void*h=dlopen("libbankcrypto.so",RTLD_NOW|RTLD_LOCAL);if(!h){fprintf(stderr,"dlopen failed: %s\n",dlerror());return 2;}dlerror();version_fn version=(version_fn)dlsym(h,"crypto_version");const char*e=dlerror();if(e){fprintf(stderr,"dlsym version failed: %s\n",e);dlclose(h);return 3;}encrypt_fn encrypt=(encrypt_fn)dlsym(h,"encrypt_demo");e=dlerror();if(e){fprintf(stderr,"dlsym encrypt failed: %s\n",e);dlclose(h);return 3;}printf("runtime-loaded version=%s checksum=%d\n",version(),encrypt("bank-record-42"));dlclose(h);return 0;}
