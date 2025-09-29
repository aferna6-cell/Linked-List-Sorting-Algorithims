/* ids_support.c
 * Aidan Fernandes
 * aferna6
 * ECE 2230 Fall 2025
 * MP2/MP3 Support
 *
 * Purpose: Support functions for the IDS alert system using the list ADT.
 * Notes:
 *   - ids_record_fill now checks fgets() return values (no warnings).
 *   - Adds MP3 fast helpers: ids_append_rear_fast, ids_sort_gen, ids_sort_ip.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "llist.h"
#include "ids_support.h"

#define MAXLINE 256

/* private helpers */
static void ids_record_fill(alert_t *rec);      /* collect input from user */
static void ids_print_alert_rec(alert_t *rec);  /* print one record */

/* ===== Comparators / matchers ===== */

int ids_compare_genid(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a && rec_b);
    if (rec_a->generator_id < rec_b->generator_id) return 1;
    if (rec_a->generator_id > rec_b->generator_id) return -1;
    return 0;
}

int ids_compare_destip(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a && rec_b);
    if (rec_a->dest_ip_addr < rec_b->dest_ip_addr) return 1;
    if (rec_a->dest_ip_addr > rec_b->dest_ip_addr) return -1;
    return 0;
}

int ids_match_destip(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a && rec_b);
    return (rec_a->dest_ip_addr == rec_b->dest_ip_addr) ? 0 : 1;
}

/* ===== Printing ===== */

void ids_print(llist_t *list_ptr, const char *list_type)
{
    assert(strcmp(list_type, "List")==0 || strcmp(list_type, "Queue")==0);
    int n = llist_entries(list_ptr);

    if (n == 0) {
        printf("%s is empty\n", list_type);
    } else {
        printf("%s contains %d record%s\n", list_type, n, n==1 ? "." : "s.");
    }
    for (int i = 0; i < n; i++) {
        printf("%d: ", i+1);
        alert_t *rec_ptr = llist_access(list_ptr, i);
        ids_print_alert_rec(rec_ptr);
    }
    printf("\n");
}

/* ===== Queue ops (interactive MP2 path) ===== */

void ids_add_rear(llist_t *list_ptr)
{
    alert_t *new_ptr = (alert_t *) calloc(1, sizeof(alert_t));
    assert(new_ptr);
    ids_record_fill(new_ptr);

    alert_t key; memset(&key, 0, sizeof(key));
    key.dest_ip_addr = new_ptr->dest_ip_addr;
    int pos = -1;
    alert_t *found = llist_elem_find(list_ptr, &key, &pos, ids_match_destip);

    if (found != NULL) {
        alert_t *old = llist_remove(list_ptr, pos);
        free(old);
        printf("Appended %d onto queue and removed old copy\n", new_ptr->dest_ip_addr);
    } else {
        printf("Appended %d onto queue\n", new_ptr->dest_ip_addr);
    }
    llist_insert(list_ptr, new_ptr, LLPOSITION_BACK);
}

void ids_remove_front(llist_t *list_ptr)
{
    alert_t *rec_ptr = llist_remove(list_ptr, LLPOSITION_FRONT);
    if (rec_ptr) {
        printf("Deleted front with IP addr: %d\n", rec_ptr->dest_ip_addr);
        free(rec_ptr);
    } else {
        printf("Queue empty, did not remove\n");
    }
}

/* ===== Create (sorted vs queue) ===== */

llist_t *ids_create(const char *list_type)
{
    assert(strcmp(list_type, "List")==0 || strcmp(list_type, "Queue")==0);
    if (strcmp(list_type, "List") == 0)  return llist_construct(ids_compare_genid);
    if (strcmp(list_type, "Queue") == 0) return llist_construct(NULL);
    printf("ERROR, invalid list type %s\n", list_type);
    exit(1);
}

/* ===== Sorted-list helpers (interactive) ===== */

void ids_add(llist_t *list_ptr)
{
    alert_t *new_ptr = (alert_t *) calloc(1, sizeof(alert_t));
    assert(new_ptr);
    ids_record_fill(new_ptr);
    llist_insert_sorted(list_ptr, new_ptr);
    printf("Inserted %d into list\n", new_ptr->generator_id);
}

void ids_list_gen(llist_t *list_ptr, int gen_id)
{
    int shown = 0, n = llist_entries(list_ptr);
    for (int i = 0; i < n; i++) {
        alert_t *rec_ptr = llist_access(list_ptr, i);
        if (rec_ptr->generator_id == gen_id) {
            ids_print_alert_rec(rec_ptr);
            shown++;
        }
    }
    if (shown) printf("Found %d alerts matching generator %d\n", shown, gen_id);
    else       printf("Did not find alert: %d\n", gen_id);
}

void ids_list_ip(llist_t *list_ptr, int dest_ip)
{
    int shown = 0, n = llist_entries(list_ptr);
    for (int i = 0; i < n; i++) {
        alert_t *rec_ptr = llist_access(list_ptr, i);
        if (rec_ptr->dest_ip_addr == dest_ip) {
            ids_print_alert_rec(rec_ptr);
            shown++;
        }
    }
    if (shown) printf("Found %d alerts matching IP %d\n", shown, dest_ip);
    else       printf("Did not find destination IP: %d\n", dest_ip);
}

void ids_remove_gen(llist_t *list_ptr, int gen_id)
{
    int removed = 0, i = 0;
    while (i < llist_entries(list_ptr)) {
        alert_t *rec_ptr = llist_access(list_ptr, i);
        if (rec_ptr->generator_id == gen_id) {
            alert_t *dead = llist_remove(list_ptr, i);
            free(dead);
            removed++;
        } else {
            i++;
        }
    }
    if (removed) printf("Removed %d alerts matching generator %d\n", removed, gen_id);
    else         printf("Did not remove alert with generator: %d\n", gen_id);
}

void ids_remove_ip(llist_t *list_ptr, int dest_ip)
{
    int removed = 0, i = 0;
    while (i < llist_entries(list_ptr)) {
        alert_t *rec_ptr = llist_access(list_ptr, i);
        if (rec_ptr->dest_ip_addr == dest_ip) {
            alert_t *dead = llist_remove(list_ptr, i);
            free(dead);
            removed++;
        } else {
            i++;
        }
    }
    if (removed) printf("Removed %d alerts matching IP %d\n", removed, dest_ip);
    else         printf("Did not remove alert with IP: %d\n", dest_ip);
}

void ids_scan(llist_t *list_ptr, int thresh)
{
    int sets = 0;
    int n = llist_entries(list_ptr);
    int *seen = (int *) calloc(n, sizeof(int));
    assert(seen);

    for (int i = 0; i < n; i++) {
        if (seen[i]) continue;
        alert_t *rec_i = llist_access(list_ptr, i);
        int g = rec_i->generator_id;
        int cnt = 1;
        seen[i] = 1;
        for (int j = i+1; j < n; j++) {
            if (seen[j]) continue;
            alert_t *rec_j = llist_access(list_ptr, j);
            if (rec_j->generator_id == g) { cnt++; seen[j] = 1; }
        }
        if (cnt >= thresh) { printf("A set with generator %d has %d alerts\n", g, cnt); sets++; }
    }
    free(seen);

    if (sets) printf("Scan found %d sets\n", sets);
    else      printf("Scan found no alerts with >= %d matches\n", thresh);
}

void ids_stats(llist_t *sorted, llist_t *unsorted)
{
    printf("Number records in list: %d, queue size: %d\n",
           llist_entries(sorted), llist_entries(unsorted));
}

void ids_cleanup(llist_t *list_ptr)
{
    llist_destruct(list_ptr);
}

/* ===== Robust input (no ignored fgets return) ===== */

static void ids_record_fill(alert_t *rec)
{
    char line[MAXLINE];
    assert(rec);

    printf("Generator component:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->generator_id);

    printf("Signature:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->signature_id);

    printf("Revision:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->revision_id);

    printf("Dest IP address:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->dest_ip_addr);

    printf("Source IP address:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->src_ip_addr);

    printf("Destination port number:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->dest_port_num);

    printf("Source port number:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->src_port_num);

    printf("Time:");
    if (!fgets(line, MAXLINE, stdin)) { fprintf(stderr, "EOF on input\n"); exit(1); }
    sscanf(line, "%d", &rec->timestamp);

    printf("\n");
}

/* print one alert */
static void ids_print_alert_rec(alert_t *rec)
{
    assert(rec);
    printf("[%d:%d:%d] (gen, sig, rev): ", rec->generator_id, rec->signature_id, rec->revision_id);
    printf("Dest IP: %d, Src: %d, Dest port: %d,", rec->dest_ip_addr, rec->src_ip_addr, rec->dest_port_num);
    printf(" Src: %d, Time: %d\n", rec->src_port_num, rec->timestamp);
}

/* ===== MP3 fast-path helpers (used by lab3 / geninput / longrun.sh) ===== */

void ids_append_rear_fast(llist_t *list_ptr, int generator_id, int dest_ip_addr)
{
    /* Build minimal record: only fields required by comparators. */
    alert_t *rec = (alert_t *) calloc(1, sizeof(alert_t));
    assert(rec);
    rec->generator_id = generator_id;
    rec->dest_ip_addr = dest_ip_addr;
    llist_insert(list_ptr, rec, LLPOSITION_BACK);
    /* No printing; matches generator’s expected quiet behavior. */
}

void ids_sort_gen(llist_t *list_ptr, int sort_type)
{
    /* Sort the list by generator id using the ADT’s sort */
    llist_sort(list_ptr, sort_type, ids_compare_genid);
}

void ids_sort_ip(llist_t *list_ptr, int sort_type)
{
    /* Sort the list by destination IP using the ADT’s sort */
    llist_sort(list_ptr, sort_type, ids_compare_destip);
}

/* vi:set ts=8 sts=4 sw=4 et: */
