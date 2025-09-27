/* lab3.c
 * Aidan Fernandes
 * aferna6
 * ECE 2230 Fall 2025
 * MP3 main program
 *
 * Purpose:
 *   Minimal driver for MP3 performance testing. Reads commands from stdin.
 *   Commands used by the provided geninput and scripts:
 *     - APPENDREAR g d    : append record with generator_id=g, dest_ip=d (no print)
 *     - SORTGEN t         : sort queue by generator_id ASC using algorithm t=1..5
 *     - SORTIP  t         : sort queue by destination IP DESC using algorithm t=1..5
 *     - PRINTQ            : print queue contents (for small N)
 *     - QUIT              : free all memory and exit
 *
 * Notes:
 *   - All interactive prompts, banners, and stray messages are suppressed.
 *   - The *only* mandatory output for timing is the single line emitted by
 *     ids_sort_{gen,ip}:  "<n>\t<msec>\t<type>\n"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ids_support.h"

#define MAXLINE 256

int main(void)
{
    char line[MAXLINE];
    llist_t *queue = ids_create("Queue");

    while (fgets(line, MAXLINE, stdin) != NULL) {
        /* skip blank/comment lines quietly */
        if (line[0] == '\n' || line[0] == '#') continue;

        /* Parse command keyword */
        char cmd[64] = {0};
        int a=0, b=0;
        if (sscanf(line, "%63s", cmd) != 1) continue;

        if (strcmp(cmd, "APPENDREAR") == 0) {
            if (sscanf(line, "%*s %d %d", &a, &b) == 2) {
                ids_append_rear_fast(queue, a, b);
            }
            /* no prints */
        } else if (strcmp(cmd, "SORTGEN") == 0) {
            if (sscanf(line, "%*s %d", &a) == 1) {
                ids_sort_gen(queue, a);   /* prints timing line */
            }
        } else if (strcmp(cmd, "SORTIP") == 0) {
            if (sscanf(line, "%*s %d", &a) == 1) {
                ids_sort_ip(queue, a);    /* prints timing line */
            }
        } else if (strcmp(cmd, "PRINTQ") == 0) {
            ids_print(queue, "Queue");
        } else if (strcmp(cmd, "QUIT") == 0) {
            ids_cleanup(queue);
            return 0;
        } else {
            /* silently ignore other commands in MP3 */
        }
    }

    /* If input ends without QUIT, still clean up for valgrind hygiene. */
    ids_cleanup(queue);
    return 0;
}
