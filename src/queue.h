#ifndef list_h
#define list_h

struct queue;

// Initialize a queue strcture
struct queue *queue_init();
void queue_free(struct queue *q);

// Add and get free elements
void queue_add_free(struct queue *q, void *data);
void *queue_get_free(struct queue *q);

// Push and pop elements
void queue_push(struct queue *q, void *data);
void *queue_pop(struct queue *q);

// Misc
int queue_length(struct queue *q);
#endif
