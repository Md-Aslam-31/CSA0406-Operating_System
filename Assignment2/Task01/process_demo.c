#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void show_proc(pid_t pid) {
    char path[64], line[256];
    snprintf(path, sizeof path, "/proc/%ld/status", (long)pid);
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return; }
    printf("/proc snapshot for pid=%ld:\n", (long)pid);
    while (fgets(line, sizeof line, f))
        if (!strncmp(line, "Name:", 5) || !strncmp(line, "State:", 6) ||
            !strncmp(line, "Pid:", 4) || !strncmp(line, "PPid:", 5) ||
            !strncmp(line, "VmSize:", 7) || !strncmp(line, "VmRSS:", 6)) fputs(line, stdout);
    fclose(f);
}

int main(void) {
    pid_t children[3];
    puts("parent: creating three observable child processes");
    for (int i = 0; i < 3; ++i) {
        pid_t p = fork();
        if (p < 0) { perror("fork"); return 1; }
        if (p == 0) {
            printf("child[%d]: pid=%ld ppid=%ld entering short work\n", i,
                   (long)getpid(), (long)getppid());
            fflush(stdout);
            struct timespec pause = {0, 250000000L * (i + 1)};
            nanosleep(&pause, NULL);
            _exit(10 + i);
        }
        children[i] = p;
    }
    puts("process snapshot (ps):");
    char command[128];
    snprintf(command, sizeof command, "ps -o pid,ppid,state,comm --ppid %ld", (long)getpid());
    (void)system(command);
    for (int i = 0; i < 3; ++i) show_proc(children[i]);
    for (int i = 0; i < 3; ++i) {
        int status;
        if (waitpid(children[i], &status, 0) < 0) { perror("waitpid"); return 1; }
        printf("parent: reaped pid=%ld exit=%d\n", (long)children[i], WEXITSTATUS(status));
    }
    return 0;
}
