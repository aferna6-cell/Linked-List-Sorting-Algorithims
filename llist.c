/* llist.c
 * Aidan Fernandes
 * aferna6
 * ECE 2230 Fall 2025
 * MP3 — Two-way Linked List ADT + Sorting
 *
 * Purpose:
 *   Generic two-way linked list ADT used by MP2/MP3, plus llist_sort()
 *   implementing five sorting algorithms for a list of data_t*.
 *
 * Note:
 *   llist_debug_validate() is kept for API compatibility but is a no-op to
 *   avoid O(N) walks that slow down long runs.
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <assert.h>
#include "llist.h"

/* Private sorted-state flags */
#define LLIST_SORTED    989898
#define LLIST_UNSORTED  -898989

/* ===== private helpers ===== */
static int  comes_before(llist_t *L, const data_t *a, const data_t *b);

static void detach_node(llist_t *L, llist_elem_t *node);
static void push_back_node(llist_t *L, llist_elem_t *node);
static llist_elem_t *pop_front_node(llist_t *L);

static void selection_sort_iter(llist_t *work, llist_t *out);
static void selection_sort_recur(llist_t *work, llist_t *out);

static void split_in_half(llist_t *src, llist_t *left, llist_t *right);
static void merge_into(llist_t *dst, llist_t *left, llist_t *right);

static int qsort_compare(const void *p_a, const void *p_b, void * lptr);

/* ===== core ADT functions ===== */

data_t *llist_access(llist_t *list_ptr, int pos_index)
{
    assert(list_ptr);
    if (list_ptr->ll_entry_count == 0) return NULL;

    if (pos_index == LLPOSITION_FRONT || pos_index == 0)
        return list_ptr->ll_front->data_ptr;
    if (pos_index == LLPOSITION_BACK || pos_index == list_ptr->ll_entry_count - 1)
        return list_ptr->ll_back->data_ptr;
    if (pos_index < 0 || pos_index >= list_ptr->ll_entry_count)
        return NULL;

    llist_elem_t *r;
    if (pos_index <= list_ptr->ll_entry_count / 2) {
        r = list_ptr->ll_front;
        for (int i = 0; i < pos_index; i++) r = r->ll_next;
    } else {
        r = list_ptr->ll_back;
        for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) r = r->ll_prev;
    }
    assert(r && r->data_ptr);
    return r->data_ptr;
}

llist_t *llist_construct(int (*fcomp)(const data_t *, const data_t *))
{
    llist_t *L = (llist_t *) malloc(sizeof(llist_t));
    assert(L);
    L->ll_front = NULL;
    L->ll_back = NULL;
    L->ll_entry_count = 0;
    L->compare_fun = fcomp;
    L->ll_sorted_state = (fcomp ? LLIST_SORTED : LLIST_UNSORTED);
    return L;
}

void llist_destruct(llist_t *list_ptr)
{
    assert(list_ptr);
    llist_elem_t *cur = list_ptr->ll_front;
    while (cur) {
        llist_elem_t *nxt = cur->ll_next;
        free(cur->data_ptr);
        free(cur);
        cur = nxt;
    }
    free(list_ptr);
}

data_t *llist_elem_find(llist_t *list_ptr, data_t *elem_ptr, int *pos_index,
                        int (*fcomp)(const data_t *, const data_t *))
{
    assert(list_ptr && pos_index);
    *pos_index = -1;
    int idx = 0;
    for (llist_elem_t *r = list_ptr->ll_front; r; r = r->ll_next, idx++) {
        if (fcomp(elem_ptr, r->data_ptr) == 0) {
            *pos_index = idx;
            return r->data_ptr;
        }
    }
    return NULL;
}

void llist_insert(llist_t *list_ptr, data_t *elem_ptr, int pos_index)
{
    assert(list_ptr);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);

    llist_elem_t *node = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    assert(node);
    node->data_ptr = elem_ptr;
    node->ll_next = node->ll_prev = NULL;

    if (list_ptr->ll_entry_count == 0) {
        list_ptr->ll_front = list_ptr->ll_back = node;
    } else if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        node->ll_next = list_ptr->ll_front;
        list_ptr->ll_front->ll_prev = node;
        list_ptr->ll_front = node;
    } else if (pos_index == LLPOSITION_BACK || pos_index >= list_ptr->ll_entry_count) {
        node->ll_prev = list_ptr->ll_back;
        list_ptr->ll_back->ll_next = node;
        list_ptr->ll_back = node;
    } else {
        llist_elem_t *r;
        if (pos_index <= list_ptr->ll_entry_count / 2) {
            r = list_ptr->ll_front;
            for (int i = 0; i < pos_index; i++) r = r->ll_next;
        } else {
            r = list_ptr->ll_back;
            for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) r = r->ll_prev;
        }
        node->ll_next = r;
        node->ll_prev = r->ll_prev;
        if (r->ll_prev) r->ll_prev->ll_next = node; else list_ptr->ll_front = node;
        r->ll_prev = node;
    }
    list_ptr->ll_entry_count++;
    if (list_ptr->ll_sorted_state == LLIST_SORTED) list_ptr->ll_sorted_state = LLIST_UNSORTED;
}

void llist_insert_sorted(llist_t *list_ptr, data_t *elem_ptr)
{
    assert(list_ptr && list_ptr->compare_fun && list_ptr->ll_sorted_state == LLIST_SORTED);

    llist_elem_t *node = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    assert(node);
    node->data_ptr = elem_ptr;
    node->ll_next = node->ll_prev = NULL;

    if (list_ptr->ll_entry_count == 0) {
        list_ptr->ll_front = list_ptr->ll_back = node;
    } else {
        llist_elem_t *r = list_ptr->ll_front;
        llist_elem_t *before = NULL;
        while (r) {
            if (list_ptr->compare_fun(elem_ptr, r->data_ptr) == 1) { before = r; break; }
            r = r->ll_next;
        }
        if (!before) {
            node->ll_prev = list_ptr->ll_back;
            list_ptr->ll_back->ll_next = node;
            list_ptr->ll_back = node;
        } else {
            node->ll_next = before;
            node->ll_prev = before->ll_prev;
            if (before->ll_prev) before->ll_prev->ll_next = node; else list_ptr->ll_front = node;
            before->ll_prev = node;
        }
    }
    list_ptr->ll_entry_count++;
}

data_t *llist_remove(llist_t *list_ptr, int pos_index)
{
    assert(list_ptr);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);

    if (list_ptr->ll_entry_count == 0) return NULL;

    llist_elem_t *t = NULL;

    if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        t = list_ptr->ll_front;
        list_ptr->ll_front = t->ll_next;
        if (list_ptr->ll_front) list_ptr->ll_front->ll_prev = NULL;
        if (list_ptr->ll_back == t) list_ptr->ll_back = NULL;
    } else if (pos_index == LLPOSITION_BACK || pos_index == list_ptr->ll_entry_count - 1) {
        t = list_ptr->ll_back;
        list_ptr->ll_back = t->ll_prev;
        if (list_ptr->ll_back) list_ptr->ll_back->ll_next = NULL;
        if (list_ptr->ll_front == t) list_ptr->ll_front = NULL;
    } else {
        if (pos_index < 0 || pos_index >= list_ptr->ll_entry_count) return NULL;
        if (pos_index <= list_ptr->ll_entry_count / 2) {
            t = list_ptr->ll_front;
            for (int i = 0; i < pos_index; i++) t = t->ll_next;
        } else {
            t = list_ptr->ll_back;
            for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) t = t->ll_prev;
        }
        if (t->ll_prev) t->ll_prev->ll_next = t->ll_next; else list_ptr->ll_front = t->ll_next;
        if (t->ll_next) t->ll_next->ll_prev = t->ll_prev; else list_ptr->ll_back = t->ll_prev;
    }

    list_ptr->ll_entry_count--;
    data_t *ret = t->data_ptr;
    free(t);
    return ret;
}

int llist_entries(llist_t *list_ptr)
{
    assert(list_ptr && list_ptr->ll_entry_count >= 0);
    return list_ptr->ll_entry_count;
}

/* ===== sorting ===== */

void llist_sort(llist_t *list_ptr, int sort_type,
                int (*fcomp)(const data_t *, const data_t *))
{
    assert(list_ptr && fcomp);
    list_ptr->compare_fun = fcomp;

    int n = llist_entries(list_ptr);
    if (n <= 1) { list_ptr->ll_sorted_state = LLIST_SORTED; llist_debug_validate(list_ptr); return; }

    switch (sort_type) {
        case 1: { /* insertion into second sorted list */
            llist_t *S = llist_construct(fcomp);
            data_t *x;
            while ((x = llist_remove(list_ptr, LLPOSITION_FRONT)) != NULL) {
                llist_insert_sorted(S, x);
            }
            list_ptr->ll_front = S->ll_front;
            list_ptr->ll_back  = S->ll_back;
            list_ptr->ll_entry_count = S->ll_entry_count;
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(S);
            break;
        }
        case 2: { /* selection sort (recursive) */
            llist_t *OUT = llist_construct(fcomp);
            selection_sort_recur(list_ptr, OUT);
            list_ptr->ll_front = OUT->ll_front;
            list_ptr->ll_back = OUT->ll_back;
            list_ptr->ll_entry_count = OUT->ll_entry_count;
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(OUT);
            break;
        }
        case 3: { /* selection sort (iterative) */
            llist_t *OUT = llist_construct(fcomp);
            selection_sort_iter(list_ptr, OUT);
            list_ptr->ll_front = OUT->ll_front;
            list_ptr->ll_back = OUT->ll_back;
            list_ptr->ll_entry_count = OUT->ll_entry_count;
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(OUT);
            break;
        }
        case 4: { /* merge sort */
            llist_t *L = llist_construct(fcomp);
            llist_t *R = llist_construct(fcomp);
            split_in_half(list_ptr, L, R);
            if (L->ll_entry_count > 1)  llist_sort(L, 4, fcomp);
            if (R->ll_entry_count > 1)  llist_sort(R, 4, fcomp);
            merge_into(list_ptr, L, R);
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(L); free(R);
            break;
        }
        case 5: { /* quick sort via qsort_r on array copy */
            int m = llist_entries(list_ptr);
            data_t **A = (data_t **) malloc(sizeof(data_t *) * m);
            assert(A);
            for (int i = 0; i < m; i++) A[i] = llist_remove(list_ptr, LLPOSITION_FRONT);
            qsort_r(A, m, sizeof(data_t *), qsort_compare, list_ptr);
            for (int i = 0; i < m; i++) llist_insert(list_ptr, A[i], LLPOSITION_BACK);
            free(A);
            list_ptr->ll_sorted_state = LLIST_SORTED;
            break;
        }
        default:
            list_ptr->ll_sorted_state = LLIST_SORTED;
            break;
    }
    llist_debug_validate(list_ptr); /* no-op for speed */
}

/* ===== helpers ===== */

static int comes_before(llist_t *L, const data_t *a, const data_t *b)
{
    assert(L->compare_fun);
    return (L->compare_fun(a, b) == 1);
}

static void detach_node(llist_t *L, llist_elem_t *node)
{
    if (node->ll_prev) node->ll_prev->ll_next = node->ll_next;
    else               L->ll_front = node->ll_next;
    if (node->ll_next) node->ll_next->ll_prev = node->ll_prev;
    else               L->ll_back  = node->ll_prev;
    node->ll_prev = node->ll_next = NULL;
    L->ll_entry_count--;
}

static void push_back_node(llist_t *L, llist_elem_t *node)
{
    node->ll_next = NULL;
    node->ll_prev = L->ll_back;
    if (L->ll_back) L->ll_back->ll_next = node; else L->ll_front = node;
    L->ll_back = node;
    L->ll_entry_count++;
}

static llist_elem_t *pop_front_node(llist_t *L)
{
    if (!L->ll_front) return NULL;
    llist_elem_t *node = L->ll_front;
    L->ll_front = node->ll_next;
    if (L->ll_front) L->ll_front->ll_prev = NULL; else L->ll_back = NULL;
    node->ll_next = node->ll_prev = NULL;
    L->ll_entry_count--;
    return node;
}

static void selection_sort_iter(llist_t *work, llist_t *out)
{
    while (work->ll_entry_count > 0) {
        llist_elem_t *best = work->ll_front;
        for (llist_elem_t *r = best->ll_next; r; r = r->ll_next)
            if (comes_before(out, r->data_ptr, best->data_ptr)) best = r;
        detach_node(work, best);
        push_back_node(out, best);
    }
}

static void selection_sort_recur(llist_t *work, llist_t *out)
{
    if (work->ll_entry_count == 0) return;
    llist_elem_t *best = work->ll_front;
    for (llist_elem_t *r = best->ll_next; r; r = r->ll_next)
        if (comes_before(out, r->data_ptr, best->data_ptr)) best = r;
    detach_node(work, best);
    push_back_node(out, best);
    selection_sort_recur(work, out);
}

static void split_in_half(llist_t *src, llist_t *left, llist_t *right)
{
    int n = src->ll_entry_count, half = n/2;
    for (int i = 0; i < half; i++)  push_back_node(left,  pop_front_node(src));
    while (src->ll_entry_count)     push_back_node(right, pop_front_node(src));
}

static void merge_into(llist_t *dst, llist_t *left, llist_t *right)
{
    while (left->ll_entry_count && right->ll_entry_count) {
        data_t *a = left->ll_front->data_ptr;
        data_t *b = right->ll_front->data_ptr;
        if (comes_before(dst, a, b)) push_back_node(dst, pop_front_node(left));
        else                         push_back_node(dst, pop_front_node(right));
    }
    while (left->ll_entry_count)  push_back_node(dst, pop_front_node(left));
    while (right->ll_entry_count) push_back_node(dst, pop_front_node(right));
}

static int qsort_compare(const void *p_a, const void *p_b, void * lptr)
{
    llist_t *L = (llist_t *) lptr;
    return L->compare_fun(*(data_t * const *)p_b, *(data_t * const *)p_a);
}

/* ===== debug validator (no-op for speed) ===== */
void llist_debug_validate(llist_t *L) { (void)L; }

/* vi:set ts=8 sts=4 sw=4 et: */
