#ifndef list_h
#define list_h

typedef struct list list_t;

list_t *init_list();
void list_free(list_t *l);
void list_add(list_t *l, void *ptr);
void list_remove(list_t *l, void *ptr);
void *list_pop();

#endif
