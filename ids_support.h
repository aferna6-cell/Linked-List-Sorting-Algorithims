#ifndef IDS_SUPPORT_H
#define IDS_SUPPORT_H

#include "llist.h"
#include "datatypes.h"   /* defines alert_t (aka data_t) */

/* Comparators / matchers */
int ids_compare_genid(const alert_t *rec_a, const alert_t *rec_b);
int ids_compare_destip(const alert_t *rec_a, const alert_t *rec_b);
int ids_match_destip(const alert_t *rec_a, const alert_t *rec_b);

/* MP2-style interactive helpers */
void   ids_print(llist_t *list_ptr, const char *list_type);
void   ids_add_rear(llist_t *list_ptr);
void   ids_remove_front(llist_t *list_ptr);
llist_t *ids_create(const char *list_type);
void   ids_add(llist_t *list_ptr);
void   ids_list_gen(llist_t *list_ptr, int gen_id);
void   ids_list_ip(llist_t *list_ptr, int dest_ip);
void   ids_remove_gen(llist_t *list_ptr, int gen_id);
void   ids_remove_ip(llist_t *list_ptr, int dest_ip);
void   ids_scan(llist_t *list_ptr, int thresh);
void   ids_stats(llist_t *sorted, llist_t *unsorted);
void   ids_cleanup(llist_t *list_ptr);

/* MP3 fast-path helpers (called by lab3 for scripted runs) */
void   ids_append_rear_fast(llist_t *list_ptr, int generator_id, int dest_ip_addr);
void   ids_sort_gen(llist_t *list_ptr, int sort_type);
void   ids_sort_ip(llist_t *list_ptr, int sort_type);

#endif /* IDS_SUPPORT_H */
