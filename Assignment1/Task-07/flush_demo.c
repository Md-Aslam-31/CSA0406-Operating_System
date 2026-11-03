#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void)
{
    printf("shell: running job 2\n");
    /* Deliberately omit fflush(stdout) to demonstrate a copied stdio buffer. */
    pid_t pid = fork();
    if (pid < 0) return 1;
    if (pid == 0) exit(3); /* exit flushes the child's inherited buffer */
    int status;
    if (waitpid(pid, &status, 0) < 0) return 2;
    return WIFEXITED(status) ? 0 : 3;
}
