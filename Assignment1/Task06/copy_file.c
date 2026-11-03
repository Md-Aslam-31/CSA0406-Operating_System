#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* Copy src to dst. Returns read-call count, or -1 on error. */
long copy_file(const char *src, const char *dst, size_t bufsize)
{
    char buf[8192];
    if (bufsize == 0 || bufsize > sizeof buf) { errno = EINVAL; return -1; }
    int in = open(src, O_RDONLY);
    if (in < 0) return -1;
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { close(in); return -1; }
    long calls = 0;
    ssize_t n;
    int failed = 0;
    for (;;) {
        do { n = read(in, buf, bufsize); } while (n < 0 && errno == EINTR);
        if (n < 0) { failed = 1; break; }
        ++calls;
        if (n == 0) break;
        ssize_t done = 0;
        while (done < n) {
            ssize_t w = write(out, buf + done, (size_t)(n - done));
            if (w < 0 && errno == EINTR) continue;
            if (w <= 0) { failed = 1; break; }
            done += w;
        }
        if (failed) break;
    }
    int saved_errno = errno;
    if (close(in) < 0) failed = 1;
    if (close(out) < 0) failed = 1;
    if (failed) { errno = saved_errno ? saved_errno : EIO; return -1; }
    return calls;
}

int main(void)
{
    FILE *f = fopen("records.dat", "w");
    if (!f) { perror("fopen"); return 1; }
    for (int i = 0; i < 10000; ++i) fputc('A' + i % 26, f);
    if (fclose(f) != 0) { perror("fclose"); return 1; }
    const size_t sizes[] = {1, 512, 4096};
    int rc = 0;
    for (size_t i = 0; i < sizeof sizes / sizeof sizes[0]; ++i) {
        long calls = copy_file("records.dat", "copy.dat", sizes[i]);
        if (calls < 0) { perror("copy_file"); rc = 1; break; }
        printf("buffer %4zu bytes: %5ld read() calls\n", sizes[i], calls);
    }
    unlink("records.dat");
    unlink("copy.dat");
    return rc;
}
