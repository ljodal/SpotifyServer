#ifndef playlist_h
#define playlist_h

#include <libspotify/api.h>
#include <jansson.h>

typedef struct playlist playlist_t;

playlist_t *playlist_init(sp_playlistcontainer *pc);
json_t *playlist_json(playlist_t *pl);

#endif
