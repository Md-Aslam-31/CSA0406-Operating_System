#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const uint64_t file_bytes = UINT64_C(4) * 1024 * 1024 * 1024;
    const uint64_t block_bytes = 4 * 1024;
    const uint64_t blocks = file_bytes / block_bytes;
    const uint64_t pointers_per_block = block_bytes / sizeof(uint32_t);
    const uint64_t inode_blocks = 12 + pointers_per_block +
        pointers_per_block * pointers_per_block +
        pointers_per_block * pointers_per_block * pointers_per_block;
    const uint64_t disk_bytes = UINT64_C(4000000000000); /* decimal 4 TB */
    const uint64_t disk_blocks = disk_bytes / block_bytes;
    const uint64_t bitmap_bytes = (disk_blocks + 7) / 8;
    const uint64_t fat_bytes = disk_blocks * sizeof(uint32_t);

    printf("4 GiB file / 4 KiB blocks: %" PRIu64 " data blocks\n", blocks);
    printf("Last-block access (inode/directory metadata excluded): contiguous=1 data read; linked=%" PRIu64 " block reads; FAT-in-memory=%" PRIu64 " RAM next-pointer lookups + 1 data read; two-level indexed=2 index reads + 1 data read\n",
           blocks, blocks - 1);
    printf("4-byte inode pointers per 4 KiB block: %" PRIu64 "\n", pointers_per_block);
    printf("12 direct + single + double + triple indirect: %" PRIu64 " blocks = %.0f bytes = %.2f TiB (theoretical pointer-address limit)\n",
           inode_blocks, (double)inode_blocks * (double)block_bytes,
           (double)inode_blocks * (double)block_bytes / (1024.0 * 1024 * 1024 * 1024));
    printf("4 TB decimal disk, 4 KiB blocks: %" PRIu64 " whole blocks\n", disk_blocks);
    printf("free-space bitmap: %" PRIu64 " bytes = %.2f MiB\n", bitmap_bytes,
           (double)bitmap_bytes / (1024.0 * 1024));
    printf("4-byte FAT for that many blocks: %" PRIu64 " bytes = %.2f MiB\n", fat_bytes,
           (double)fat_bytes / (1024.0 * 1024));
    return 0;
}
