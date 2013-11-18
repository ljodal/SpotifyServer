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

void playqueue_delete(playqueue_t *q, uint32_t i) {

    // Remove the element from the list
    uint32_t l;
    sp_track *track = (sp_track *)arraylist_remove(q->al, &l, i);

    // Release the track
    sp_track_release(track);
}

void playqueue_move(playqueue_t *q, uint32_t from, uint32_t to) {
    uint32_t queue_size = arraylist_size(q->al);

    // Indexes must be less than the queue_size
    if (from >= queue_size || to >= queue_size) return;

    // Indexes can't be the same
    if (from == to) return;

    // Remove the element from the list
    uint32_t l;
    sp_track *track = (sp_track *)arraylist_remove(q->al, &l, from);

    // Insert the track at the new location
    arraylist_add(q->al, (void *)track, l, to);
}
