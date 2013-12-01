#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arraylist.h"

typedef struct
{
    void *data;
    size_t len;
} arraylist_e;

struct arraylist
{
    // The current capacity of the array
    unsigned int size;

    // The current number of objects in the array
    unsigned int count;

    // The size we started out with
    unsigned int initial_size;

    // The first object in the array
    arraylist_e *array;
};

/**************************************************
 *
 * These functions are private functions that
 * the library uses internally to handle the
 * array list.
 *
 **************************************************/

static int expand_array(arraylist_t *a)
{
#ifdef DEBUG
    fprintf(stderr, "Expanding array\n");
#endif
    void *tmp = realloc(a->array, sizeof(arraylist_e) * a->size * 2);

    if (!tmp)
        return -1;

    a->array = tmp;

    a->size *= 2;

    return 0;
}

static int contract_array(arraylist_t *a)
{
#ifdef DEBUG
    fprintf(stderr, "Contracting array\n");
#endif
    void *tmp = realloc(a->array, sizeof(arraylist_e) * a->size / 2);

    if (!tmp)
        return -1;

    a->array = tmp;
    a->size /= 2;

    return 0;
}

/**************************************************
 *
 * These are public functions that make up the api.
 *
 **************************************************/

// Initialize and return a pointer to a new array list
arraylist_t *arraylist_init(unsigned int size)
{
    arraylist_t *a = malloc(sizeof(arraylist_t));
    a->size = size;
    a->initial_size = size;
    a->count = 0;
    a->array = malloc(size * sizeof(arraylist_e));

    if (!a->array)
        return NULL;

    memset(a->array, 0, size*sizeof(arraylist_e));
#ifdef DEBUG
    fprintf(stderr, "Size: %d, count: %d\n", a->size, a->count);
#endif
    return a;
}

/*
 * Free all memory used by the arraylist
 */
void arraylist_free(arraylist_t *a)
{
    free(a->array);
    free(a);
}

// Add a new element to the end of the list
int arraylist_push(arraylist_t *a, void *d, unsigned int l)
{
#ifdef DEBUG
    fprintf(stderr, "Adding at %d\n", a->count);
#endif
    return arraylist_add(a, d, l, a->count);
}

void *arraylist_pop(arraylist_t *a, unsigned int *l)
{
    return arraylist_remove(a, l, 0);
}

int arraylist_add(arraylist_t *a, void *d, unsigned int l, unsigned int i)
{
    // Ignore empty elements
    if (!a)
        return -1;

    // Make sure we have capacity
    if (a->size == a->count && expand_array(a) != 0)
        return -1;

    // Move other elements
    if (i < a->count) {
        memmove(&a->array[i+1], &a->array[i], sizeof(arraylist_e) * (a->count - i));
    }

    a->array[i].data = d;
    a->array[i].len = l;

    a->count++;

#ifdef DEBUG
    fprintf(stderr, "Size: %d, count: %d\n", a->size, a->count);
#endif

    return 0;
}

void *arraylist_remove(arraylist_t *a, unsigned int *l, unsigned int i)
{
    if (a->count == 0 || a->count <= i) {
        *l = 0;
        return NULL;
    }

    void *d = a->array[i].data;
    *l = a->array[i].len;

    memmove(&a->array[i], &a->array[i+1], sizeof(arraylist_e) * (a->count - i - 1));

    if (a->count / 2 > a->initial_size && a->count < a->size / 2)
        contract_array(a);

    a->count--;

    return d;
}

// Return the number of elements in the array list
uint32_t arraylist_size(arraylist_t *a) {
    return a->count;
}

// Look at an element without deleting it
void *arraylist_peek(arraylist_t *a, uint32_t *l, uint32_t i) {
    *l = a->array[i].len;
    return a->array[i].data;
}
