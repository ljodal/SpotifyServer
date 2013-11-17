#include <stdlib.h>
#include <stdio.h>

#include "arraylist.h"
#include "play_queue.h"

struct playqueue
{
    arraylist_t *al;
};

playqueue_t *playqueue_init(size_t size)
{
    playqueue_t *q = malloc(sizeof(playqueue_t));
    q->al = arraylist_init(size);
    return q;
}

int playqueue_add(playqueue_t *q, sp_track *track)
{
    fprintf(stderr, "Queueing new track\n");
    sp_track_add_ref(track);
    return arraylist_push(q->al, (void *)track, 0);
}

sp_track *playqueue_next(playqueue_t *q)
{
    unsigned int l;
    sp_track *track = (sp_track *)arraylist_pop(q->al, &l);

    return track;
}

void playqueue_to_json(playqueue_t *q, json_t *array) {
    uint32_t l;
    for (int i = 0; i < arraylist_size(q->al); i++) {
        sp_track *track = (sp_track *)arraylist_peek(q->al, &l, i);

        json_t *object = json_object();
        if (sp_track_is_loaded(track)) {
            json_object_set_new(object, "loaded", json_true());
        } else {
            json_object_set_new(object, "loaded", json_false());
        }
        json_object_set_new(object, "name", json_string(sp_track_name(track)));

        json_array_append_new(array, object);
    }
}
