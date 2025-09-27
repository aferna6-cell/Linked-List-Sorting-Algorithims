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
 * MP3 notes:
 *   - Only llist_sort() calls llist_debug_validate() (at the end).
 *     All other validation calls are removed for performance.
 *   - llist_sort() sets list_ptr->compare_fun = fcomp and
 *     list_ptr->ll_sorted_state = LLIST_SORTED before returning.
 *
 * Bugs: None known.
 */
#define _GNU_SOURCE   /* so stdlib.h declares qsort_r */

#include <stdlib.h>
#include <assert.h>
#include "llist.h"

/* Private sorted-state flags (kept inside llist.c). */
#define LLIST_SORTED    989898
#define LLIST_UNSORTED  -898989

/* ===== Forward declarations for private helpers ===== */
static int  comes_before(llist_t *L, const data_t *a, const data_t *b);

static void detach_node(llist_t *L, llist_elem_t *node);
static void push_back_node(llist_t *L, llist_elem_t *node);
static llist_elem_t *pop_front_node(llist_t *L);

static void selection_sort_iter(llist_t *work, llist_t *out);
static void selection_sort_recur(llist_t *work, llist_t *out);

static void split_in_half(llist_t *src, llist_t *left, llist_t *right);
static void merge_into(llist_t *dst, llist_t *left, llist_t *right);

static int qsort_compare(const void *p_a, const void *p_b, void * lptr);

/* llist_debug_validate is declared in llist.h and defined at the end. */

/* ===== Core ADT functions (MP2) ===== */

data_t *llist_access(llist_t *list_ptr, int pos_index) {
    assert(list_ptr != NULL);

    if (list_ptr->ll_entry_count == 0) return NULL;

    if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        return list_ptr->ll_front->data_ptr;
    }
    if (pos_index == LLPOSITION_BACK || pos_index == list_ptr->ll_entry_count - 1) {
        return list_ptr->ll_back->data_ptr;
    }
    if (pos_index < 0 || pos_index >= list_ptr->ll_entry_count) {
        return NULL;
    }

    llist_elem_t *rover;
    if (pos_index <= list_ptr->ll_entry_count / 2) {
        rover = list_ptr->ll_front;
        for (int i = 0; i < pos_index; i++) rover = rover->ll_next;
    } else {
        rover = list_ptr->ll_back;
        for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) rover = rover->ll_prev;
    }
    assert(rover != NULL && rover->data_ptr != NULL);
    return rover->data_ptr;
}

llist_t *llist_construct(int (*fcomp)(const data_t *, const data_t *)) {
    llist_t *L = (llist_t *) malloc(sizeof(llist_t));
    assert(L != NULL);
    L->ll_front = NULL;
    L->ll_back = NULL;
    L->ll_entry_count = 0;
    L->compare_fun = fcomp;
    L->ll_sorted_state = (fcomp == NULL ? LLIST_UNSORTED : LLIST_SORTED);
    return L;
}

void llist_destruct(llist_t *list_ptr) {
    assert(list_ptr != NULL);
    llist_elem_t *cur = list_ptr->ll_front;
    while (cur != NULL) {
        llist_elem_t *next = cur->ll_next;
        free(cur->data_ptr);          /* free user record */
        free(cur);
        cur = next;
    }
    free(list_ptr);
}

data_t *llist_elem_find(llist_t *list_ptr, data_t *elem_ptr, int *pos_index,
                        int (*fcomp)(const data_t *, const data_t *)) {
    assert(list_ptr != NULL);
    assert(pos_index != NULL);

    *pos_index = -1;
    int idx = 0;
    for (llist_elem_t *r = list_ptr->ll_front; r != NULL; r = r->ll_next, idx++) {
        if (fcomp(elem_ptr, r->data_ptr) == 0) {
            *pos_index = idx;
            return r->data_ptr;
        }
    }
    return NULL;
}

void llist_insert(llist_t *list_ptr, data_t *elem_ptr, int pos_index) {
    assert(list_ptr != NULL);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);

    llist_elem_t *node = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    assert(node != NULL);
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
        llist_elem_t *rover;
        if (pos_index <= list_ptr->ll_entry_count / 2) {
            rover = list_ptr->ll_front;
            for (int i = 0; i < pos_index; i++) rover = rover->ll_next;
        } else {
            rover = list_ptr->ll_back;
            for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) rover = rover->ll_prev;
        }
        node->ll_next = rover;
        node->ll_prev = rover->ll_prev;
        if (rover->ll_prev != NULL) rover->ll_prev->ll_next = node;
        else list_ptr->ll_front = node;
        rover->ll_prev = node;
    }

    list_ptr->ll_entry_count++;
    if (list_ptr->ll_sorted_state == LLIST_SORTED) {
        list_ptr->ll_sorted_state = LLIST_UNSORTED;
    }
}

void llist_insert_sorted(llist_t *list_ptr, data_t *elem_ptr) {
    assert(list_ptr != NULL);
    assert(list_ptr->compare_fun != NULL);
    assert(list_ptr->ll_sorted_state == LLIST_SORTED);

    llist_elem_t *node = (llist_elem_t *) malloc(sizeof(llist_elem_t));
    assert(node != NULL);
    node->data_ptr = elem_ptr;
    node->ll_next = node->ll_prev = NULL;

    if (list_ptr->ll_entry_count == 0) {
        list_ptr->ll_front = list_ptr->ll_back = node;
    } else {
        llist_elem_t *rover = list_ptr->ll_front;
        llist_elem_t *insert_before = NULL;
        while (rover != NULL) {
            int cmp = list_ptr->compare_fun(elem_ptr, rover->data_ptr);
            if (cmp == 1) {  /* new BEFORE rover per compare_fun */
                insert_before = rover;
                break;
            }
            rover = rover->ll_next;   /* cmp == 0 or -1 => keep moving (stable) */
        }

        if (insert_before == NULL) {
            node->ll_prev = list_ptr->ll_back;
            list_ptr->ll_back->ll_next = node;
            list_ptr->ll_back = node;
        } else {
            node->ll_next = insert_before;
            node->ll_prev = insert_before->ll_prev;
            if (insert_before->ll_prev != NULL) insert_before->ll_prev->ll_next = node;
            else list_ptr->ll_front = node;
            insert_before->ll_prev = node;
        }
    }
    list_ptr->ll_entry_count++;
    /* still sorted */
}

data_t *llist_remove(llist_t *list_ptr, int pos_index) {
    assert(list_ptr != NULL);
    assert(pos_index == LLPOSITION_FRONT || pos_index == LLPOSITION_BACK || pos_index >= 0);

    if (list_ptr->ll_entry_count == 0) return NULL;

    llist_elem_t *target = NULL;

    if (pos_index == LLPOSITION_FRONT || pos_index == 0) {
        target = list_ptr->ll_front;
        list_ptr->ll_front = target->ll_next;
        if (list_ptr->ll_front != NULL) list_ptr->ll_front->ll_prev = NULL;
        if (list_ptr->ll_back == target) list_ptr->ll_back = NULL;
    } else if (pos_index == LLPOSITION_BACK || pos_index == list_ptr->ll_entry_count - 1) {
        target = list_ptr->ll_back;
        list_ptr->ll_back = target->ll_prev;
        if (list_ptr->ll_back != NULL) list_ptr->ll_back->ll_next = NULL;
        if (list_ptr->ll_front == target) list_ptr->ll_front = NULL;
    } else {
        if (pos_index < 0 || pos_index >= list_ptr->ll_entry_count) return NULL;
        if (pos_index <= list_ptr->ll_entry_count / 2) {
            target = list_ptr->ll_front;
            for (int i = 0; i < pos_index; i++) target = target->ll_next;
        } else {
            target = list_ptr->ll_back;
            for (int i = list_ptr->ll_entry_count - 1; i > pos_index; i--) target = target->ll_prev;
        }
        if (target->ll_prev != NULL) target->ll_prev->ll_next = target->ll_next;
        else list_ptr->ll_front = target->ll_next;
        if (target->ll_next != NULL) target->ll_next->ll_prev = target->ll_prev;
        else list_ptr->ll_back = target->ll_prev;
    }

    list_ptr->ll_entry_count--;
    data_t *ret = target->data_ptr;
    free(target);
    return ret;
}

int llist_entries(llist_t *list_ptr) {
    assert(list_ptr != NULL);
    assert(list_ptr->ll_entry_count >= 0);
    return list_ptr->ll_entry_count;
}

/* ===== Sorting (MP3) ===== */

void llist_sort(llist_t *list_ptr, int sort_type,
                int (*fcomp)(const data_t *, const data_t *)) {
    assert(list_ptr != NULL);
    assert(fcomp != NULL);

    /* set the comparator immediately for all algorithms */
    list_ptr->compare_fun = fcomp;

    int n = llist_entries(list_ptr);
    if (n <= 1) {
        list_ptr->ll_sorted_state = LLIST_SORTED;
        llist_debug_validate(list_ptr);
        return;
    }

    switch (sort_type) {
        case 1: { /* Insertion sort: move into second sorted list */
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
        case 2: { /* Recursive Selection Sort (pointer version of Standish 5.19) */
            llist_t *OUT = llist_construct(fcomp);
            selection_sort_recur(list_ptr, OUT);
            list_ptr->ll_front = OUT->ll_front;
            list_ptr->ll_back = OUT->ll_back;
            list_ptr->ll_entry_count = OUT->ll_entry_count;
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(OUT);
            break;
        }
        case 3: { /* Iterative Selection Sort (Standish 5.35) */
            llist_t *OUT = llist_construct(fcomp);
            selection_sort_iter(list_ptr, OUT);
            list_ptr->ll_front = OUT->ll_front;
            list_ptr->ll_back = OUT->ll_back;
            list_ptr->ll_entry_count = OUT->ll_entry_count;
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(OUT);
            break;
        }
        case 4: { /* Merge Sort (recursive) */
            llist_t *LEFT  = llist_construct(fcomp);
            llist_t *RIGHT = llist_construct(fcomp);
            split_in_half(list_ptr, LEFT, RIGHT);   /* empties list_ptr */

            if (LEFT->ll_entry_count > 1)  llist_sort(LEFT, 4, fcomp);
            if (RIGHT->ll_entry_count > 1) llist_sort(RIGHT, 4, fcomp);

            merge_into(list_ptr, LEFT, RIGHT);
            list_ptr->ll_sorted_state = LLIST_SORTED;
            free(LEFT);
            free(RIGHT);
            break;
        }
        case 5: { /* Quick Sort via qsort_r on array of data_t* */
            int Asize = llist_entries(list_ptr);
            data_t **QsortA = (data_t **) malloc(sizeof(data_t *) * Asize);
            assert(QsortA != NULL);
            for (int i = 0; i < Asize; i++) {
                QsortA[i] = llist_remove(list_ptr, LLPOSITION_FRONT);
            }
            qsort_r(QsortA, Asize, sizeof(data_t *), qsort_compare, list_ptr);
            for (int i = 0; i < Asize; i++) {
                llist_insert(list_ptr, QsortA[i], LLPOSITION_BACK);
            }
            free(QsortA);
            list_ptr->ll_sorted_state = LLIST_SORTED;
            break;
        }
        default:
            /* Unknown type: mark sorted to keep invariants consistent. */
            list_ptr->ll_sorted_state = LLIST_SORTED;
            break;
    }

    /* Final required validator call (keep only here for performance). */
    llist_debug_validate(list_ptr);
}

/* ===== Private helpers ===== */

static int comes_before(llist_t *L, const data_t *a, const data_t *b) {
    /* true iff a should appear BEFORE b according to compare_fun. */
    assert(L->compare_fun != NULL);
    return (L->compare_fun(a, b) == 1);
}

static void detach_node(llist_t *L, llist_elem_t *node) {
    if (node->ll_prev) node->ll_prev->ll_next = node->ll_next;
    else               L->ll_front = node->ll_next;

    if (node->ll_next) node->ll_next->ll_prev = node->ll_prev;
    else               L->ll_back = node->ll_prev;

    node->ll_prev = node->ll_next = NULL;
    L->ll_entry_count--;
}

static void push_back_node(llist_t *L, llist_elem_t *node) {
    node->ll_next = NULL;
    node->ll_prev = L->ll_back;
    if (L->ll_back) L->ll_back->ll_next = node;
    else            L->ll_front = node;
    L->ll_back = node;
    L->ll_entry_count++;
}

static llist_elem_t *pop_front_node(llist_t *L) {
    if (L->ll_front == NULL) return NULL;
    llist_elem_t *node = L->ll_front;
    L->ll_front = node->ll_next;
    if (L->ll_front) L->ll_front->ll_prev = NULL;
    else             L->ll_back = NULL;
    node->ll_next = node->ll_prev = NULL;
    L->ll_entry_count--;
    return node;
}

/* Build OUT by repeatedly removing the minimum node from WORK and pushing to OUT back. */
static void selection_sort_iter(llist_t *work, llist_t *out) {
    while (work->ll_entry_count > 0) {
        llist_elem_t *best = work->ll_front;
        for (llist_elem_t *r = best->ll_next; r != NULL; r = r->ll_next) {
            if (comes_before(out, r->data_ptr, best->data_ptr)) best = r;
        }
        detach_node(work, best);
        push_back_node(out, best);
    }
}

/* Recursive version: move one minimum per call from WORK to OUT until WORK empty. */
static void selection_sort_recur(llist_t *work, llist_t *out) {
    if (work->ll_entry_count == 0) return;
    llist_elem_t *best = work->ll_front;
    for (llist_elem_t *r = best->ll_next; r != NULL; r = r->ll_next) {
        if (comes_before(out, r->data_ptr, best->data_ptr)) best = r;
    }
    detach_node(work, best);
    push_back_node(out, best);
    selection_sort_recur(work, out);
}

/* Split SRC into LEFT and RIGHT lists of (roughly) equal size. */
static void split_in_half(llist_t *src, llist_t *left, llist_t *right) {
    int n = src->ll_entry_count;
    int half = n / 2;
    for (int i = 0; i < half; i++) {
        llist_elem_t *node = pop_front_node(src);
        push_back_node(left, node);
    }
    while (src->ll_entry_count > 0) {
        llist_elem_t *node = pop_front_node(src);
        push_back_node(right, node);
    }
}

/* Merge two already-sorted lists (LEFT, RIGHT) into DST (which must be empty). */
static void merge_into(llist_t *dst, llist_t *left, llist_t *right) {
    while (left->ll_entry_count > 0 && right->ll_entry_count > 0) {
        data_t *a = left->ll_front->data_ptr;
        data_t *b = right->ll_front->data_ptr;
        if (comes_before(dst, a, b)) {
            push_back_node(dst, pop_front_node(left));
        } else {
            push_back_node(dst, pop_front_node(right));
        }
    }
    while (left->ll_entry_count > 0)  push_back_node(dst, pop_front_node(left));
    while (right->ll_entry_count > 0) push_back_node(dst, pop_front_node(right));
}

/* qsort_r comparator: double-dereference and reverse order per StandishSort.c template. */
static int qsort_compare(const void *p_a, const void *p_b, void * lptr) {
    llist_t *list_ptr = (llist_t *) lptr;
    return list_ptr->compare_fun(*(data_t * const *) p_b, *(data_t * const *) p_a);
}

/* ===== Debug validator (required by grader/linker) ===== */
void llist_debug_validate(llist_t *L) {
    llist_elem_t *N;
    int count = 0;
    assert(L != NULL);
    if (L->ll_front == NULL)
        assert(L->ll_back == NULL && L->ll_entry_count == 0);
    if (L->ll_back == NULL)
        assert(L->ll_front == NULL && L->ll_entry_count == 0);
    if (L->ll_entry_count == 0)
        assert(L->ll_front == NULL && L->ll_back == NULL);
    if (L->ll_entry_count == 1) {
        assert(L->ll_front == L->ll_back && L->ll_front != NULL);
        assert(L->ll_front->ll_next == NULL && L->ll_front->ll_prev == NULL);
        assert(L->ll_front->data_ptr != NULL);
    }
    if (L->ll_front == L->ll_back && L->ll_front != NULL)
        assert(L->ll_entry_count == 1);
    assert(L->ll_sorted_state == LLIST_SORTED || L->ll_sorted_state == LLIST_UNSORTED);
    if (L->ll_entry_count > 1) {
        assert(L->ll_front != L->ll_back && L->ll_front != NULL && L->ll_back != NULL);
        N = L->ll_front;
        assert(N->ll_prev == NULL);
        while (N != NULL) {
            assert(N->data_ptr != NULL);
            if (N->ll_next != NULL) assert(N->ll_next->ll_prev == N);
            else assert(N == L->ll_back);
            count++;
            N = N->ll_next;
        }
        assert(count == L->ll_entry_count);
    }
    if (L->ll_sorted_state == LLIST_SORTED && L->ll_front != NULL) {
        N = L->ll_front;
        while (N->ll_next != NULL) {
            /* ensure each element <= next element according to compare_fun */
            assert(L->compare_fun(N->data_ptr, N->ll_next->data_ptr) != -1);
            N = N->ll_next;
        }
    }
}

/* vi:set ts=8 sts=4 sw=4 et: */
