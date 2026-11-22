#!/usr/bin/env bash
set -euo pipefail
mkdir -p evidence
{
  echo '=== Host filesystem containing the assignment ==='
  findmnt -T . -o TARGET,SOURCE,FSTYPE,OPTIONS
  sample=$(mktemp)
  printf 'assignment-4-filesystem-inspection\n' > "$sample"
  stat -c 'stat: inode=%i size=%s bytes block_size=%o blocks=%b mode=%A' "$sample"
  echo 'df -i for working filesystem:'
  df -i .
  echo 'ls -i sample:'
  ls -i "$sample"
  rm -f "$sample"
  src=$(findmnt -n -o SOURCE -T .)
  echo "tune2fs attempt for host source: $src"
  if command -v tune2fs >/dev/null 2>&1 && [[ -b "$src" ]]; then
    sudo -n tune2fs -l "$src" 2>&1 | head -n 25 || true
  else
    echo 'Host mount is not a directly accessible ext filesystem block device in this runner.'
  fi
  echo '=== Created ext4 image inspected with dumpe2fs ==='
  img=$(mktemp --suffix=.img)
  truncate -s 64M "$img"
  mkfs.ext4 -F -q "$img"
  dumpe2fs -h "$img" 2>&1 | grep -E 'Filesystem UUID|Filesystem features|Block count|Free blocks|Block size|Inode count|Free inodes|Inode size|Journal features|Checksum' || true
  rm -f "$img"
} | tee evidence/task03-filesystem.txt
