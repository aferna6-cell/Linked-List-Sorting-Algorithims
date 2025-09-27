/* llist.h
 * List ADT interface for MP2/MP3
 */
#ifndef LLIST_H
#define LLIST_H

#include "datatypes.h"

/* Special index values for head/tail operations */
#define LLPOSITION_FRONT   (-987654)
#define LLPOSITION_BACK    (-234567)

/* Node and header (opaque to users of the ADT) */
typedef struct llist_element_tag {
    data_t *data_ptr;
    struct llist_element_tag *ll_prev;
    struct llist_element_tag *ll_next;
} llist_elem_t;

typedef struct llist_header_tag {
    llist_elem_t *ll_front;
    llist_elem_t *ll_back;
    int ll_entry_count;
    int ll_sorted_state;  /* private flag used only by llist.c */
    int (*compare_fun)(const data_t *, const data_t *);
} llist_t;

/* MP2 functions */
data_t *  llist_access(llist_t *list_ptr, int pos_index);
llist_t * llist_construct(int (*fcomp)(const data_t *, const data_t *));
void      llist_destruct(llist_t *list_ptr);
data_t *  llist_elem_find(llist_t *list_ptr, data_t *elem_ptr, int *pos_index,
                          int (*fcomp)(const data_t *, const data_t *));
int       llist_entries(llist_t *list_ptr);
void      llist_insert(llist_t *list_ptr, data_t *elem_ptr, int pos_index);
void      llist_insert_sorted(llist_t *list_ptr, data_t *elem_ptr);
data_t *  llist_remove(llist_t *list_ptr, int pos_index);

/* MP3 sorting */
void      llist_sort(llist_t *list_ptr, int sort_type,
                     int (*fcomp)(const data_t *, const data_t *));

/* Debug validator provided by template (do not remove decl). */
void      llist_debug_validate(llist_t *L);

#endif
