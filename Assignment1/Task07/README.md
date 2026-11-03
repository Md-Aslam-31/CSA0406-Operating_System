# Task 7 — Process-control program

Compile on Linux:

```sh
gcc -std=c11 -Wall -Wextra -pthread shell_jobs.c -o shell_jobs
./shell_jobs
```

Expected output:

```text
shell: running job 1
42
job 1 exit status 0
shell: running job 2
job 2 exit status 3
```

The child executes `expr 6 * 7`; the parent waits and reports its normal exit status. The second child exits with status 3. `fflush(stdout)` before each fork ensures buffered output is not duplicated into the child. The program has not been run on Linux in the local environment. CI is configured to execute it on Ubuntu and preserve text output as an artifact.
