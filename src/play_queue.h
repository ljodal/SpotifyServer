#ifndef play_queue_h
#define play_queue_h

#include <libspotify/api.h>
#include <jansson.h>

typedef struct playqueue playqueue_t;

playqueue_t *playqueue_init(size_t size);
int playqueue_add(playqueue_t *q, sp_track *track);
sp_track *playqueue_next(playqueue_t *q);

void playqueue_to_json(playqueue_t *q, json_t *array);
void playqueue_delete(playqueue_t *q, uint32_t i);
void playqueue_move(playqueue_t *q, uint32_t from, uint32_t to);

#endif
