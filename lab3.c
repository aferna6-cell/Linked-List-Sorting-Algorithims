/* lab3.c
 * Aidan Fernandes (aferna6)
 * ECE 2230 Fall 2025
 * MP3 main program
 *
 * Minimal driver for MP3 performance testing. Reads commands from stdin.
 * Commands used by geninput/longrun.sh:
 *   - APPENDREAR g d   : append record with generator_id=g, dest_ip=d (no print)
 *   - SORTGEN t        : sort queue by generator_id ASC using algorithm t=1..5
 *   - SORTIP  t        : sort queue by dest_ip     DESC using algorithm t=1..5
 *   - PRINTQ           : print queue contents (for small N)
 *   - QUIT             : free all memory and exit
 *
 * The ONLY mandatory output for timing is the single line:
 *     "<N>\t<msec>\t<type>\n"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "ids_support.h"   /* includes llist.h / datatypes.h */

#define MAXLINE 256

static double ms_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

int main(void)
{
    char line[MAXLINE];
    llist_t *queue = ids_create("Queue");

    while (fgets(line, MAXLINE, stdin) != NULL) {

        /* skip blank/comment lines quietly */
        if (line[0] == '\n' || line[0] == '#') continue;

        /* parse first token */
        char cmd[64] = {0};
        if (sscanf(line, "%63s", cmd) != 1) continue;

        if (strcmp(cmd, "APPENDREAR") == 0) {
            int gen, ip;
            if (sscanf(line, "%*s %d %d", &gen, &ip) == 2) {
                ids_append_rear_fast(queue, gen, ip);
            }

        } else if (strcmp(cmd, "SORTGEN") == 0) {
            int t;
            if (sscanf(line, "%*s %d", &t) == 1) {
                int N = llist_entries(queue);
                double t0 = ms_now();
                ids_sort_gen(queue, t);
                double t1 = ms_now();
                printf("%d\t%.6f\t%d\n", N, t1 - t0, t);
                fflush(stdout);
            }

        } else if (strcmp(cmd, "SORTIP") == 0) {
            int t;
            if (sscanf(line, "%*s %d", &t) == 1) {
                int N = llist_entries(queue);
                double t0 = ms_now();
                ids_sort_ip(queue, t);
                double t1 = ms_now();
                printf("%d\t%.6f\t%d\n", N, t1 - t0, t);
                fflush(stdout);
            }

        } else if (strcmp(cmd, "PRINTQ") == 0) {
            ids_print(queue, "Queue");

        } else if (strcmp(cmd, "QUIT") == 0) {
            ids_cleanup(queue);
            return 0;

        } else {
            /* silently ignore any other commands */
        }
    }

    /* If input ends without QUIT, still clean up for valgrind hygiene. */
    ids_cleanup(queue);
    return 0;
}
