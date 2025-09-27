/* ids_support.h
 * Support functions for IDS alert records used by MP2/MP3.
 */
#ifndef IDS_SUPPORT_H
#define IDS_SUPPORT_H

#include "llist.h"
#include "datatypes.h"

/* Comparators (do not access inside llist.c) */
int ids_compare_genid(const alert_t *rec_a, const alert_t *rec_b);
/* For MP3: sort by destination IP in DESCENDING order */
int ids_compare_destip_desc(const alert_t *rec_a, const alert_t *rec_b);

/* Matching predicate for queue duplicate (MP2) */
int ids_match_destip(const alert_t *rec_a, const alert_t *rec_b);

/* Create sorted or unsorted list */
llist_t *ids_create(const char *list_type);

/* Printing and cleanup utilities */
void ids_print(llist_t *list_ptr, const char *list_type);
void ids_stats(llist_t *sorted, llist_t *unsorted);
void ids_cleanup(llist_t *list_ptr);

/* MP2 queue/list operations (still useful in MP3 small runs) */
void ids_add(llist_t *list_ptr);
void ids_add_rear(llist_t *list_ptr);
void ids_remove_front(llist_t *list_ptr);
void ids_list_gen(llist_t *list_ptr, int gen_id);
void ids_list_ip(llist_t *list_ptr, int dest_ip);
void ids_remove_gen(llist_t *list_ptr, int gen_id);
void ids_remove_ip(llist_t *list_ptr, int dest_ip);

/* MP3 additions */
void ids_append_rear_fast(llist_t *queue_ptr, int gen_id, int dest_ip);
void ids_sort_gen(llist_t *queue_ptr, int sort_type);
void ids_sort_ip(llist_t *queue_ptr,  int sort_type);

#endif
