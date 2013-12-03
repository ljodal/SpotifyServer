#include "list.h"

#include <stdlib.h>
#include <stdio.h>

struct list_e
{
    void *ptr;
    struct list_e *next;
    struct list_e *prev;
};

struct list
{
    struct list_e *first;
    struct list_e *last;
    size_t count;
};

/*
 * Initialize a new list
 */
list_t *init_list()
{
    list_t *l = malloc(sizeof(list_t));
    l->first = NULL;
    l->last = NULL;
    l->count = 0;

    return l;
}

/*
 * Add an element to the list
 */
void list_add(list_t *l, void *ptr)
{
    struct list_e *el = calloc(1, sizeof(struct list_e));
    el->ptr = ptr;
    if (l->count == 0) {
        l->first = el;
        l->last = el;
    } else {
        l->last->next = el;
        el->prev = l->last;
        l->last = el;
    }

    // Increase counter
    l->count += 1;
}

/*
 * Remove an element from the list
 */
void list_remove(list_t *l, void *ptr)
{
    struct list_e *el = l->first;
    while(el && el->ptr != ptr) {
        el = el->next;
    }

    if (el) {
        if (el->prev)
            el->prev->next = el->next;
        if (el->next)
            el->next->prev = el->prev;

        if (el == l->first)
            l->first = el->next;

        if (el == l->last)
            l->last = el->prev;

        free(el);

        l->count -= 1;
    } else {
        fprintf(stderr, "Element not found in the list\n");
    }
}

/*
 * Pop the first element from the list
 */
void *list_pop(list_t *l)
{
    void *ptr = l->first->ptr;

    struct list_e *el = l->first;

    l->first = el->next;
    l->first->prev = NULL;

    free(el);

    return ptr;
}

/*
 * Free the list
 */
void list_free(list_t *l)
{
    // Remove all elements in the list
    void *el;
    while((el = list_pop(l)) != NULL) {}

    // Free the list
    free(l);
}
