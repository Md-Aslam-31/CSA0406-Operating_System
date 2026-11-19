#include <stdio.h>
#ifndef VERSION
#define VERSION "1.0"
#endif
const char *crypto_version(void) { return VERSION; }
int encrypt_demo(const char *input) { unsigned sum=0; for (const unsigned char *p=(const unsigned char *)input; *p; ++p) sum=(sum*33u)^*p; return (int)sum; }
