#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CYLINDERS 200
#define REQUESTS 10000
#define SEEK_MS_PER_CYL 0.08
#define IO_MS 2.0

enum policy { FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK, POLICY_COUNT };
static const char *policy_name[] = {"FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};
struct request { int cyl, state; double arrival; };
struct stats { double avg_move, max_wait, avg_wait; };
static uint32_t rng = 0x0406a55aU;
static uint32_t random_u32(void)
{
    rng = rng * 1664525U + 1013904223U;
    return rng;
}
static void add_arrivals(struct request r[], int *next, double now)
{
    while (*next < REQUESTS && r[*next].arrival <= now) {
        r[*next].state = 1;
        (*next)++;
    }
}
static int queued(const struct request r[], int count)
{
    for (int i = 0; i < count; i++) if (r[i].state == 1) return 1;
    return 0;
}
static int choose_fifo(const struct request r[])
{
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) return i;
    return -1;
}
static int choose_sstf(const struct request r[], int head)
{
    int best = -1, distance = CYLINDERS + 1;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int d = abs(r[i].cyl - head);
        if (d < distance) { best = i; distance = d; }
    }
    return best;
}
static int choose_direction(const struct request r[], int head, int direction, int extreme)
{
    int best = -1;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int c = r[i].cyl;
        if ((direction > 0 && c >= head) || (direction < 0 && c <= head)) {
            if (best < 0 || (direction > 0 ? c < r[best].cyl : c > r[best].cyl)) best = i;
        }
    }
    if (best >= 0 || !extreme) return best;
    for (int i = 0; i < REQUESTS; i++) if (r[i].state == 1) {
        int c = r[i].cyl;
        if (best < 0 || (direction > 0 ? c < r[best].cyl : c > r[best].cyl)) best = i;
    }
    return best;
}
static struct stats simulate(enum policy policy, double interval)
{
    struct request *r = calloc(REQUESTS, sizeof *r);
    if (!r) exit(2);
    rng = 0x0406a55aU;
    for (int i = 0; i < REQUESTS; i++) {
        r[i].cyl = (int)(random_u32() % CYLINDERS);
        r[i].arrival = i * interval;
    }
    int next = 0, head = 53, direction = policy == SCAN ? -1 : 1;
    double now = 0, total_move = 0, total_wait = 0, max_wait = 0;
    for (int served = 0; served < REQUESTS;) {
        add_arrivals(r, &next, now);
        if (!queued(r, REQUESTS)) {
            now = r[next].arrival;
            add_arrivals(r, &next, now);
        }
        int pick = -1;
        if (policy == FCFS) pick = choose_fifo(r);
        else if (policy == SSTF) pick = choose_sstf(r, head);
        else if (policy == SCAN || policy == CSCAN || policy == LOOK || policy == CLOOK) {
            pick = choose_direction(r, head, direction, 0);
            if (pick < 0 && (policy == LOOK || policy == CLOOK)) {
                int other = 0;
                for (int j = 0; j < REQUESTS; j++) if (r[j].state == 1) other = 1;
                if (other && policy == LOOK) {
                    direction = -direction;
                    pick = choose_direction(r, head, direction, 0);
                } else if (other && policy == CLOOK) {
                    direction = 1;
                    int far = -1;
                    for (int j = 0; j < REQUESTS; j++) if (r[j].state == 1 &&
                        (far < 0 || r[j].cyl < r[far].cyl)) far = j;
                    pick = far;
                    if (pick >= 0) {
                        total_move += abs(head - r[pick].cyl);
                        now += abs(head - r[pick].cyl) * SEEK_MS_PER_CYL;
                        head = r[pick].cyl;
                    }
                }
            } else if (pick < 0 && policy == SCAN) {
                int edge = direction < 0 ? 0 : CYLINDERS - 1;
                total_move += abs(head - edge);
                now += abs(head - edge) * SEEK_MS_PER_CYL;
                head = edge; direction = -direction;
                add_arrivals(r, &next, now);
                pick = choose_direction(r, head, direction, 0);
            } else if (pick < 0 && policy == CSCAN) {
                int edge = direction > 0 ? CYLINDERS - 1 : 0;
                int wrap = direction > 0 ? 0 : CYLINDERS - 1;
                total_move += abs(head - edge) + abs(edge - wrap);
                now += (abs(head - edge) + abs(edge - wrap)) * SEEK_MS_PER_CYL;
                head = wrap;
                add_arrivals(r, &next, now);
                pick = choose_direction(r, head, direction, 1);
            }
        }
        if (pick < 0 && next < REQUESTS && policy >= SCAN) {
            if ((policy == LOOK || policy == CLOOK) && !queued(r, REQUESTS)) {
                now = r[next].arrival;
                add_arrivals(r, &next, now);
            }
            continue;
        }
        if (pick < 0) { free(r); fprintf(stderr, "scheduler made no progress\n"); exit(3); }
        double wait = now - r[pick].arrival;
        if (wait > max_wait) max_wait = wait;
        total_wait += wait;
        int distance = abs(head - r[pick].cyl);
        total_move += distance;
        now += distance * SEEK_MS_PER_CYL + IO_MS;
        head = r[pick].cyl;
        r[pick].state = 2;
        served++;
    }
    struct stats result = {total_move / REQUESTS, max_wait, total_wait / REQUESTS};
    free(r);
    return result;
}
int main(void)
{
    const double intervals[] = {30.0, 10.0, 3.0};
    const char *loads[] = {"light", "medium", "heavy"};
    puts("Synthetic online queue simulation: 10,000 seeded requests, cylinders 0-199, head=53");
    puts("Model: 0.08 ms per cylinder plus 2 ms per request; arrival intervals are assumptions, not hardware benchmarks.");
    puts("load,policy,arrival_ms,average_head_movement_cyl,average_wait_ms,max_wait_ms");
    for (int l = 0; l < 3; l++) for (int p = 0; p < POLICY_COUNT; p++) {
        struct stats s = simulate((enum policy)p, intervals[l]);
        printf("%s,%s,%.1f,%.3f,%.3f,%.3f\n", loads[l], policy_name[p],
               intervals[l], s.avg_move, s.avg_wait, s.max_wait);
    }
    puts("SSTF starvation illustration: while positioned at cylinder 60, 200 requests at cylinder 60 arrive every 1 ms; each read occupies 2 ms. A queued request at 183 remains behind the closer stream for at least 400 ms. If closer work continues without bound, SSTF has no finite wait bound.");
    return 0;
}
