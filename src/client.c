#include "client.h"
#include "list.h"
#include "playlist.h"

#include <stdlib.h>
#include <string.h>

/*
 *
 * Playlist callbacks
 *
 */
void c_tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata);
void c_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata);
void c_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata);
static sp_playlist_callbacks playlist_callbacks = {
    .tracks_added = &c_tracks_added,
    .tracks_removed = &c_tracks_removed,
    .tracks_moved = &c_tracks_moved
};

struct client
{
    struct bufferevent *bev;
    list_t *playlists;
};

client_t *init_client(struct bufferevent *bev)
{
    client_t *c = malloc(sizeof(client_t));

    // Set the buffer event
    c->bev = bev;
    c->playlists = init_list();

    return c;
}

void client_playlist_on(client_t *c, sp_playlist *pl)
{
    fprintf(stderr, "Start monitoring playlist\n");
    // Add callbacks
    sp_playlist_add_callbacks(pl, &playlist_callbacks, c);

    // Add to list of playlists
    list_add(c->playlists, pl);

    // Broadcast tracks
    json_t *json = json_object();
    json_object_set_new_nocheck(json, "type", json_string_nocheck("playlist"));

    // Uri
    sp_link *l = sp_link_create_from_playlist(pl);
    char uri[200];
    sp_link_as_string(l, uri, sizeof(uri));
    json_object_set_new_nocheck(json, "user_data", json_string(uri));

    // Tracks
    json_t *tracks = json_array();

    // Loop through all tracks
    int count = sp_playlist_num_tracks(pl);
    for (int i = 0; i < count; i++) {
        json_t *track = json_object();

        // Get the track
        sp_track *t = sp_playlist_track(pl, i);

        // Track name
        json_object_set_new(track, "name", json_string(sp_track_name(t)));

        // Track uri
        sp_link *l = sp_link_create_from_track(t, 0);
        sp_link_as_string(l, uri, sizeof(uri));
        json_object_set_new_nocheck(track, "uri", json_string(uri));

        // Add the track to the array
        json_array_append_new(tracks, track);
    }

    // Add tracks to the object
    json_object_set_new(json, "tracks", tracks);

    char *msg = json_dumps(json, JSON_COMPACT);
    client_send(c, msg, strlen(msg));
    free(msg);

    json_decref(json);
}

void client_playlist_off(client_t *c, sp_playlist *pl)
{
    fprintf(stderr, "Stop monitoring playlist\n");
    // Remove callback
    sp_playlist_remove_callbacks(pl, &playlist_callbacks, c);

    // Remove from list
    list_remove(c->playlists, pl);
}

void client_free(client_t *c)
{
    // Remove all playlist callbacks
    sp_playlist *pl;
    while((pl = list_pop(c->playlists)) != NULL) {
        // Remove callback
    }

    // Free all resources held by the client
    bufferevent_free(c->bev);
    list_free(c->playlists);

    free(c);
}

void client_send(client_t* c, char *data, size_t len)
{
    bufferevent_write(c->bev, data, len);
    bufferevent_write(c->bev, (uint8_t *)"\r\n", 2);
}

void c_tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata)
{
    fprintf(stderr, "Tracks added\n");
}
void c_tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
{
    fprintf(stderr, "Tracks removed\n");
}
void c_tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
{
    fprintf(stderr, "Tracks moved\n");
}
