# Q1 - Linking, Loading and Address Binding

`../scripts/run_linux_evidence.sh` builds static and shared variants, compares size/`ldd`, uses `dlopen`/`dlsym`, records `/proc/self/maps`, replaces the shared object without relinking, tests missing-library behavior and samples two simultaneous mappings. See `../evidence/task01-linking.txt`.
