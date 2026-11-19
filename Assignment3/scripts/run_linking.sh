#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p Task01/build/lib evidence
cd Task01/build
gcc -std=c11 -Wall -Wextra -pthread -DVERSION='"1.0"' -fPIC -shared ../crypto.c -o lib/libbankcrypto.so
gcc -std=c11 -Wall -Wextra -pthread -DVERSION='"1.0"' -c ../crypto.c -o crypto.o
ar rcs lib/libbankcrypto.a crypto.o
gcc -std=c11 -Wall -Wextra -pthread ../linked_demo.c lib/libbankcrypto.a -o linked_static
gcc -std=c11 -Wall -Wextra -pthread ../linked_demo.c -Llib -lbankcrypto -Wl,-rpath,'$ORIGIN/lib' -o linked_dynamic
gcc -std=c11 -Wall -Wextra -pthread ../dlopen_demo.c -ldl -o dlopen_demo
{
  echo 'gcc -std=c11 -Wall -Wextra -pthread -DVERSION="1.0" -fPIC -shared ../crypto.c -o lib/libbankcrypto.so'
  echo 'gcc -std=c11 -Wall -Wextra -pthread -DVERSION="1.0" -c ../crypto.c -o crypto.o; ar rcs lib/libbankcrypto.a crypto.o'
  echo 'gcc -std=c11 -Wall -Wextra -pthread ../linked_demo.c lib/libbankcrypto.a -o linked_static'
  echo 'gcc -std=c11 -Wall -Wextra -pthread ../linked_demo.c -Llib -lbankcrypto -Wl,-rpath,$ORIGIN/lib -o linked_dynamic'
  echo 'gcc -std=c11 -Wall -Wextra -pthread ../dlopen_demo.c -ldl -o dlopen_demo'
  echo 'Build and linking'; ls -lh linked_static linked_dynamic lib/libbankcrypto.so
  echo 'ldd static-library variant'; ldd linked_static
  echo 'ldd shared-library variant'; ldd linked_dynamic
  echo 'Startup and maximum RSS'; /usr/bin/time -f 'static elapsed=%e s max_rss=%M KiB' ./linked_static >/dev/null
  /usr/bin/time -f 'dynamic elapsed=%e s max_rss=%M KiB' ./linked_dynamic >/dev/null
  echo 'Run-time dlopen/dlsym'; LD_LIBRARY_PATH=lib ./dlopen_demo
  echo 'Initial dynamic program'; ./linked_dynamic
  echo 'Replace shared object only (no relink)'; gcc -std=c11 -Wall -Wextra -pthread -DVERSION='"2.0"' -fPIC -shared ../crypto.c -o lib/libbankcrypto.new.so; mv lib/libbankcrypto.new.so lib/libbankcrypto.so; ./linked_dynamic
  echo 'Static executable remains at original version'; ./linked_static
  ./linked_dynamic --hold > copy1.txt & p1=$!; sleep 0.2
  ./linked_dynamic --hold > copy2.txt & p2=$!; sleep 0.2
  echo "two processes: pid1=$p1 pid2=$p2; shared library mappings:" 
  for p in "$p1" "$p2"; do echo "PID $p"; awk '/libbankcrypto.so/ {show=1; print; next} /^[[:xdigit:]]+-/ {show=0} show && /Shared_Clean|Private_Clean/ {print}' "/proc/$p/smaps" | head -n 8; done
  wait "$p1"; wait "$p2"
  echo 'Missing shared-library failure test'; mv lib/libbankcrypto.so lib/libbankcrypto.so.missing
  set +e; LD_LIBRARY_PATH=lib ./linked_dynamic > missing-linked.txt 2>&1; rc1=$?; LD_LIBRARY_PATH=lib ./dlopen_demo > missing-dlopen.txt 2>&1; rc2=$?; set -e
  mv lib/libbankcrypto.so.missing lib/libbankcrypto.so
  printf 'linked exit=%d\n' "$rc1"; cat missing-linked.txt; printf 'dlopen exit=%d\n' "$rc2"; cat missing-dlopen.txt
} > ../../evidence/task01-linking.txt 2>&1
