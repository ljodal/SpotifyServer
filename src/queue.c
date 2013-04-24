#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

/*
 * Structures
 */
struct queue_e
{
    struct queue_e *next;
    struct queue_e *prev;

    void *data;
};

struct queue
{
    struct queue_e *first;
    struct queue_e *last;
    struct queue_e *free;
    size_t length;
};

/*
 * Initialize and free
 */
struct queue *queue_init()
{
    struct queue *q = calloc(1, sizeof(struct queue));
    return q;
}

void queue_free(struct queue *q)
{
    // Remove all used elements
    struct queue_e *tmp = q->first;
    while(tmp) {
        struct queue_e *this = tmp;
        tmp = tmp->next;
        free(this->data);
        free(this);
    }

    // Remove all unused elements
    tmp = q->free;
    while(tmp) {
        struct queue_e *this = tmp;
        tmp = tmp->next;
        free(this->data);
        free(this);
    }

    // Free the queue itself
    free(q);
}

/*
 * Add and get free elements
 */
void queue_add_free(struct queue *q, void *data)
{
    // Create a new elment and add the data pointer
    struct queue_e *el = calloc(1, sizeof(struct queue_e));
    el->data = data;

    // Add the element to the free list
    if (!q->free) {
        q->free = el;
    } else {
        el->next = q->free;
        q->free = el;
    }
}

void *queue_get_free(struct queue *q)
{
    // Get the first free element
    struct queue_e *el = q->free;

    if (!el) {
        // No free elements
        return NULL;
    } else {
        // Update free list and return
        void *data = el->data;
        q->free = el->next;
        free(el);
        return data;
    }
}

/*
 * Push and pop elements
 */
void queue_push(struct queue *q, void *data)
{
    // Create a new element and add the data pointer
    struct queue_e *el = calloc(1, sizeof(struct queue_e));
    el->data = data;

    // Add the element to the end of the queue
    if (!q->first) {
        q->first = el;
        q->last = el;
    } else {
        el->prev = q->last;
        q->last->next = el;
        q->last = el;
    }

    q->length++;
}

void *queue_pop(struct queue *q)
{
    void *data = NULL;
    struct queue_e *el = q->first;
    if (el) {
        data = el->data;
        if (q->first == q->last) {
            q->last = el->next;
        }
        q->first = el->next;
        q->length--;
        free(el);
    }
    return data;
}

int queue_length(struct queue *q)
{
    return q->length;
}
