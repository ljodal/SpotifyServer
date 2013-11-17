#ifndef sl_arraylist_h
#define sl_arraylist_h

typedef struct arraylist arraylist_t;

arraylist_t *arraylist_init(unsigned int size);
void arraylist_free(arraylist_t *a);

int arraylist_push(arraylist_t *a, void *d, unsigned int l);
int arraylist_add(arraylist_t *a, void *d, unsigned int l, unsigned int i);

void *arraylist_pop(arraylist_t *a, unsigned int *l);
void *arraylist_remove(arraylist_t *a, unsigned int *l, unsigned int i);

uint32_t arraylist_size(arraylist_t *a);
void *arraylist_peek(arraylist_t *a, uint32_t *l, uint32_t i);
#endif
