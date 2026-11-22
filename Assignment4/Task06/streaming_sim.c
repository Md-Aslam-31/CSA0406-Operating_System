#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STARTS 100000
#define VIDEO_COUNT 10000
#define SEGMENT_MIB 1.0
#define DISK_COUNT 4
#define HDD_SEEK_MS 8.0
#define HDD_MIB_PER_SEC 180.0
#define SSD_SEEK_MS 0.15
#define SSD_MIB_PER_SEC 1800.0

static uint32_t rng = 0x4062026U;
static uint32_t next_u32(void) { rng = rng * 1664525U + 1013904223U; return rng; }
static int compare_double(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}
static void simulate_local_hdd(double wait[], double service)
{
    double available[DISK_COUNT] = {0};
    for (int i = 0; i < STARTS; i++) {
        int disk = 0;
        for (int d = 1; d < DISK_COUNT; d++) if (available[d] < available[disk]) disk = d;
        double arrival = i * 0.5; /* 2,000 start requests/second; time unit is ms */
        double start = fmax(arrival, available[disk]);
        wait[i] = start - arrival + service;
        available[disk] = start + service;
    }
}
static void simulate_ssd_edge_cache(double wait[], double hit_ratio)
{
    double available[DISK_COUNT] = {0};
    for (int i = 0; i < STARTS; i++) {
        int video = (int)(next_u32() % VIDEO_COUNT);
        int hot = video < (int)(VIDEO_COUNT * hit_ratio);
        double arrival = i * 0.5;
        if (hot) {
            wait[i] = 20.0 + SSD_SEEK_MS + SEGMENT_MIB / SSD_MIB_PER_SEC * 1000.0;
        } else {
            int disk = 0;
            for (int d = 1; d < DISK_COUNT; d++) if (available[d] < available[disk]) disk = d;
            double service = HDD_SEEK_MS + SEGMENT_MIB / HDD_MIB_PER_SEC * 1000.0;
            double start = fmax(arrival, available[disk]);
            wait[i] = start - arrival + 20.0 + service;
            available[disk] = start + service;
        }
    }
}
static void report(const char *name, double values[])
{
    double *sorted = malloc(sizeof(double) * STARTS);
    if (!sorted) exit(2);
    for (int i = 0; i < STARTS; i++) sorted[i] = values[i];
    qsort(sorted, STARTS, sizeof *sorted, compare_double);
    double sum = 0;
    for (int i = 0; i < STARTS; i++) sum += sorted[i];
    int reads = 0;
    for (int i = 0; i < STARTS; i++) if (values[i] > 21.0) reads++;
    printf("%s: starts=%d mean_start_latency=%.3f ms p50=%.3f ms p95=%.3f ms p99=%.3f ms max=%.3f ms origin_reads=%d/%d\n",
           name, STARTS, sum / STARTS, sorted[STARTS / 2],
           sorted[(STARTS * 95) / 100], sorted[(STARTS * 99) / 100], sorted[STARTS - 1],
           name[0] == 'L' ? STARTS : reads, STARTS);
    free(sorted);
}
int main(void)
{
    const double bytes_per_video_gib = 1.0;
    const double library_gib = VIDEO_COUNT * bytes_per_video_gib;
    const double network_gbps = 20000.0 * 4.0 / 1000.0;
    const double origin_gbps = network_gbps * 0.05;
    double service = HDD_SEEK_MS + SEGMENT_MIB / HDD_MIB_PER_SEC * 1000.0;
    double *a = calloc(STARTS, sizeof *a), *b = calloc(STARTS, sizeof *b);
    if (!a || !b) return 2;
    printf("Assumed catalog: %d videos x %.1f GiB = %.1f TiB raw; 3 replicas = %.1f TiB before overhead.\n",
           VIDEO_COUNT, bytes_per_video_gib, library_gib / 1024.0, library_gib * 3 / 1024.0);
    printf("Peak scenario: 20,000 viewers x 4 Mbit/s = %.0f Gbit/s; CDN 95%% cache ratio => %.1f Gbit/s origin egress.\n",
           network_gbps, origin_gbps);
    printf("Random-seek segment: %.1f MiB. Model service: HDD %.3f ms, SSD %.3f ms; network edge RTT 20 ms.\n",
           SEGMENT_MIB, service, SSD_SEEK_MS + SEGMENT_MIB / SSD_MIB_PER_SEC * 1000.0);
    printf("4 TB disk at 4 KiB allocation blocks: each %.1f MiB segment spans %d blocks; an offset lookup needs a few extent/index reads before the data run.\n",
           SEGMENT_MIB,
           (int)(SEGMENT_MIB * 1024.0 / 4.0));
    simulate_local_hdd(a, service);
    simulate_ssd_edge_cache(b, 0.95);
    report("Local HDD pool (4 disks, no edge cache)", a);
    report("SSD edge cache plus HDD origin (95% hits)", b);
    puts("Arrival burst is modeled at 2,000 starts/second. All latency values are a deterministic queueing simulation using stated assumptions, not a physical hardware or CDN benchmark. Playback start includes the modeled random segment lookup, not full-video transfer.");
    free(a); free(b);
    return 0;
}
