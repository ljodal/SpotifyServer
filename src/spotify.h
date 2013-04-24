#ifndef spotify_h
#define spotify_h

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
int search(char *query, void *searcher);

#endif
