#ifndef spotify_h
#define spotify_h

#include <stdint.h>

#include "client.h"

typedef enum
{
    SPOTIFY_TRACK_QUEUED,
    SPOTIFY_TRACK_NOT_QUEUED,
    SPOTIFY_ALBUM_QUEUED,
    SPOTIFY_ALBUM_NOT_QUEUED,
    SPOTIFY_PLAYLIST_QUEUED,
    SPOTIFY_PLAYLIST_NOT_QUEUED,
    SPOTIFY_INVALID_URI,
    SPOTIFY_ERROR
} spotify_status_t;

void init_spotify(const char *username, const char *password);
void spotify_loop();
int queue_link(char *);
void play_next();
int search(const char *query, void *searcher);

// Manipulate the queue
void queue_broadcast();
void queue_move(uint32_t from, uint32_t to);
void queue_delete(uint32_t i);

// Get playlists
void playlists(void *cb);

// Monitor playlists
void playlist_start(char *uri, client_t *c);
void playlist_stop(char *uri, client_t *c);

#endif
