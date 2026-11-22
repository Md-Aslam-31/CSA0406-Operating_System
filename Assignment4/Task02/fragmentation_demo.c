#include <stdio.h>
#include <string.h>

#define BLOCKS 32
static char disk[BLOCKS];
static int contiguous(int size)
{
    int run = 0;
    for (int i = 0; i < BLOCKS; i++) {
        run = disk[i] == '.' ? run + 1 : 0;
        if (run == size) return i - size + 1;
    }
    return -1;
}
static int indexed(int size, int *index, int *data)
{
    int got = 0, root = -1;
    for (int i = 0; i < BLOCKS; i++) if (disk[i] == '.') {
        if (root < 0) root = i;
        else data[got++] = i;
        if (got == size) break;
    }
    if (root < 0 || got < size) return -1;
    disk[root] = 'i'; *index = root;
    for (int i = 0; i < size; i++) disk[data[i]] = 'X';
    return 0;
}
int main(void)
{
    memset(disk, '.', sizeof disk);
    memset(disk, 'A', 5); memset(disk + 10, 'C', 5); memset(disk + 20, 'E', 5);
    printf("fragmented layout: %.32s\n", disk);
    printf("free blocks=17, largest run=7, contiguous 10-block allocation start=%d\n", contiguous(10));
    int index, data[10];
    if (indexed(10, &index, data) != 0) return 1;
    printf("indexed 10-block allocation succeeds: index block=%d, data blocks", index);
    for (int i = 0; i < 10; i++) printf(" %d", data[i]);
    putchar('\n');
    return 0;
}
