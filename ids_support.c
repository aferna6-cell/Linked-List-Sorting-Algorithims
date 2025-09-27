/* ids_support.c
 * Aidan Fernandes
 * aferna6
 * ECE 2230 Fall 2025
 * MP3
 *
 * Purpose: Support functions for IDS alert records with list ADT.
 *   MP3 adds high-speed APPENDREAR for data generation and timed sorting.
 *
 * Assumptions:
 *   - list ADT stores opaque data_t* which is typedef'd to alert_t*.
 *   - No direct access to list private members here (no ->ll_).
 *
 * Bugs: None known
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "llist.h"
#include "ids_support.h"

#define MAXLINE 256

/* Forward decls for private helpers used interactively */
static void ids_record_fill(alert_t *rec);   // collect input from user
static void ids_print_alert_rec(alert_t *rec);  // print one record

/* ===== Comparators and matchers ===== */

/* Ascending by generator_id (smaller gen ID comes BEFORE larger) */
int ids_compare_genid(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a != NULL && rec_b != NULL);
    if (rec_a->generator_id < rec_b->generator_id) return 1;
    else if (rec_a->generator_id > rec_b->generator_id) return -1;
    else return 0;
}

/* DESCENDING by destination IP (larger dest_ip BEFORE smaller) */
int ids_compare_destip_desc(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a != NULL && rec_b != NULL);
    if (rec_a->dest_ip_addr > rec_b->dest_ip_addr) return 1;
    else if (rec_a->dest_ip_addr < rec_b->dest_ip_addr) return -1;
    else return 0;
}

/* Equality on destination IP (for MP2 duplicate detection) */
int ids_match_destip(const alert_t *rec_a, const alert_t *rec_b)
{
    assert(rec_a != NULL && rec_b != NULL);
    return (rec_a->dest_ip_addr == rec_b->dest_ip_addr) ? 0 : 1;
}

/* ===== Basic utilities carried from MP2 ===== */

void ids_print(llist_t *list_ptr, const char *list_type)
{
    assert(strcmp(list_type, "List")==0 || strcmp(list_type, "Queue")==0);
    int num_in_list = llist_entries(list_ptr);
    int index;

    if (num_in_list == 0) {
        printf("%s is empty\n", list_type);
    } else {
        printf("%s contains %d record%s\n", list_type, num_in_list,
                num_in_list==1 ? "." : "s.");
    }

    for (index = 0; index < num_in_list; index++) {
        printf("%d: ", index+1);
        alert_t *rec_ptr = (alert_t *) llist_access(list_ptr, index);
        ids_print_alert_rec(rec_ptr);
    }
    printf("\n");
}

llist_t *ids_create(const char *list_type)
{
    assert(strcmp(list_type, "List")==0 || strcmp(list_type, "Queue")==0);
    if (strcmp(list_type, "List") == 0) {
        return llist_construct((int (*)(const data_t*, const data_t*)) ids_compare_genid);
    } else if (strcmp(list_type, "Queue") == 0) {
        return llist_construct(NULL);
    } else {
        printf("ERROR, invalid list type %s\n", list_type);
        exit(1);
    }
}

void ids_cleanup(llist_t *list_ptr)
{
    llist_destruct(list_ptr);
}

/* === MP2 interactive helpers (still used by PRINTQ in small tests) === */

static void ids_record_fill(alert_t *rec)
{
    char line[MAXLINE];
    assert(rec != NULL);

    printf("Generator component:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->generator_id);
    printf("Signature:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->signature_id);
    printf("Revision:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->revision_id);
    printf("Dest IP address:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->dest_ip_addr);
    printf("Source IP address:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->src_ip_addr);
    printf("Destination port number:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->dest_port_num);
    printf("Source port number:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->src_port_num);
    printf("Time:");
    fgets(line, MAXLINE, stdin);
    sscanf(line, "%d", &rec->timestamp);
    printf("\n");
}

static void ids_print_alert_rec(alert_t *rec)
{
    assert(rec != NULL);
    printf("[%d:%d:%d] (gen, sig, rev): ", rec->generator_id, rec->signature_id,
            rec->revision_id);
    printf("Dest IP: %d, Src: %d, Dest port: %d,", rec->dest_ip_addr,
            rec->src_ip_addr, rec->dest_port_num);
    printf(" Src: %d, Time: %d\n", rec->src_port_num, rec->timestamp);
}

/* ===== MP3-specific functions ===== */

/* Fast APPENDREAR: only generator_id and dest_ip_addr are set; no prints. */
void ids_append_rear_fast(llist_t *queue_ptr, int gen_id, int dest_ip)
{
    alert_t *new_ptr = (alert_t *) calloc(1, sizeof(alert_t));
    assert(new_ptr != NULL);
    new_ptr->generator_id = gen_id;
    new_ptr->dest_ip_addr = dest_ip;
    /* all other fields left as 0 by calloc */
    llist_insert(queue_ptr, (data_t *) new_ptr, LLPOSITION_BACK);
}

/* Sort by generator_id (ascending) using selected algorithm.
 * Prints: "<n>\t<elapsed_ms>\t<sort_type>\n" exactly.
 */
void ids_sort_gen(llist_t *queue_ptr, int sort_type)
{
    int n = llist_entries(queue_ptr);
    clock_t t0 = clock();
    llist_sort(queue_ptr, sort_type,
        (int (*)(const data_t*, const data_t*)) ids_compare_genid);
    clock_t t1 = clock();
    double msec = 1000.0 * (double)(t1 - t0) / (double) CLOCKS_PER_SEC;
    printf("%d\t%f\t%d\n", n, msec, sort_type);
}

/* Sort by destination IP (DESCENDING) using selected algorithm. */
void ids_sort_ip(llist_t *queue_ptr, int sort_type)
{
    int n = llist_entries(queue_ptr);
    clock_t t0 = clock();
    llist_sort(queue_ptr, sort_type,
        (int (*)(const data_t*, const data_t*)) ids_compare_destip_desc);
    clock_t t1 = clock();
    double msec = 1000.0 * (double)(t1 - t0) / (double) CLOCKS_PER_SEC;
    printf("%d\t%f\t%d\n", n, msec, sort_type);
}

/* ====== Legacy MP2 functions kept for compatibility (not used by MP3 tests) ===== */

void ids_add(llist_t *list_ptr)
{
    alert_t *new_ptr = (alert_t *) calloc(1, sizeof(alert_t));
    ids_record_fill(new_ptr);
    llist_insert_sorted(list_ptr, (data_t *) new_ptr);
    printf("Inserted %d into list\n", new_ptr->generator_id);
}

void ids_add_rear(llist_t *list_ptr)
{
    alert_t *new_ptr = (alert_t *) calloc(1, sizeof(alert_t));
    ids_record_fill(new_ptr);

    alert_t template;
    template.dest_ip_addr = new_ptr->dest_ip_addr;
    int position;
    alert_t *found_ptr = (alert_t *) llist_elem_find(list_ptr, (data_t *) &template,
                             &position, (int (*)(const data_t*,const data_t*)) ids_match_destip);

    if (found_ptr != NULL) {
        alert_t *removed = (alert_t *) llist_remove(list_ptr, position);
        free(removed);
        printf("Appended %d onto queue and removed old copy\n", new_ptr->dest_ip_addr);
    } else {
        printf("Appended %d onto queue\n", new_ptr->dest_ip_addr);
    }
    llist_insert(list_ptr, (data_t *) new_ptr, LLPOSITION_BACK);
}

void ids_remove_front(llist_t *list_ptr)
{
    alert_t *rec_ptr = (alert_t *) llist_remove(list_ptr, LLPOSITION_FRONT);
    if (rec_ptr != NULL) {
        printf("Deleted front with IP addr: %d\n", rec_ptr->dest_ip_addr);
        free(rec_ptr);
    } else {
        printf("Queue empty, did not remove\n");
    }
}

void ids_list_gen(llist_t *list_ptr, int gen_id)
{
    int count = 0;
    int num_entries = llist_entries(list_ptr);
    for (int i = 0; i < num_entries; i++) {
        alert_t *rec_ptr = (alert_t *) llist_access(list_ptr, i);
        if (rec_ptr->generator_id == gen_id) {
            ids_print_alert_rec(rec_ptr);
            count++;
        }
    }
    if (count > 0)
        printf("Found %d alerts matching generator %d\n", count, gen_id);
    else
        printf("Did not find alert: %d\n", gen_id);
}

void ids_list_ip(llist_t *list_ptr, int dest_ip)
{
    int count = 0;
    int num_entries = llist_entries(list_ptr);
    for (int i = 0; i < num_entries; i++) {
        alert_t *rec_ptr = (alert_t *) llist_access(list_ptr, i);
        if (rec_ptr->dest_ip_addr == dest_ip) {
            ids_print_alert_rec(rec_ptr);
            count++;
        }
    }
    if (count > 0)
        printf("Found %d alerts matching IP %d\n", count, dest_ip);
    else
        printf("Did not find destination IP: %d\n", dest_ip);
}

void ids_remove_gen(llist_t *list_ptr, int gen_id)
{
    int count = 0;
    int i = 0;
    while (i < llist_entries(list_ptr)) {
        alert_t *rec_ptr = (alert_t *) llist_access(list_ptr, i);
        if (rec_ptr->generator_id == gen_id) {
            alert_t *removed = (alert_t *) llist_remove(list_ptr, i);
            free(removed);
            count++;
        } else {
            i++;
        }
    }
    if (count > 0)
        printf("Removed %d alerts matching generator %d\n", count, gen_id);
    else
        printf("Did not remove alert with generator: %d\n", gen_id);
}

void ids_remove_ip(llist_t *list_ptr, int dest_ip)
{
    int count = 0;
    int i = 0;
    while (i < llist_entries(list_ptr)) {
        alert_t *rec_ptr = (alert_t *) llist_access(list_ptr, i);
        if (rec_ptr->dest_ip_addr == dest_ip) {
            alert_t *removed = (alert_t *) llist_remove(list_ptr, i);
            free(removed);
            count++;
        } else {
            i++;
        }
    }
    if (count > 0)
        printf("Removed %d alerts matching IP %d\n", count, dest_ip);
    else
        printf("Did not remove alert with IP: %d\n", dest_ip);
}

/* vi:set ts=8 sts=4 sw=4 et: */
