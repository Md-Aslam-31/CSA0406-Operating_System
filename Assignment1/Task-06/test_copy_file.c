#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define main copy_file_demo_main
#include "copy_file.c"
#undef main

int main(void)
{
    FILE *f = fopen("boundary-input.dat", "wb");
    if (!f) return 10;
    for (int i = 0; i < 10000; ++i) fputc('A' + i % 26, f);
    if (fclose(f) != 0) return 11;
    if (copy_file("boundary-input.dat", "boundary-output.dat", 4096) != 4) return 12;
    FILE *a = fopen("boundary-input.dat", "rb"), *b = fopen("boundary-output.dat", "rb");
    if (!a || !b) return 13;
    int ca, cb, mismatch = 0;
    do { ca = fgetc(a); cb = fgetc(b); if (ca != cb) mismatch = 1; } while (ca != EOF && cb != EOF);
    if (ca != cb) mismatch = 1;
    fclose(a); fclose(b);
    if (mismatch) return 14;
    errno = 0;
    if (copy_file("boundary-input.dat", "unused.dat", 0) != -1 || errno != EINVAL) return 15;
    errno = 0;
    if (copy_file("boundary-input.dat", "unused.dat", 8193) != -1 || errno != EINVAL) return 16;
    if (copy_file("missing-input.dat", "unused.dat", 4096) != -1) return 17;
    if (copy_file("boundary-input.dat", ".", 4096) != -1) return 18;
    unlink("boundary-input.dat"); unlink("boundary-output.dat"); unlink("unused.dat");
    puts("PASS: 10,000-byte copy content, zero/oversized buffers, missing source, and invalid destination");
    return 0;
}
