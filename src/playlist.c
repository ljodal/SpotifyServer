#include "playlist.h"

#include <stdlib.h>
#include <stdbool.h>

struct playlist
{
    sp_playlistcontainer *playlists;
    bool loaded;
};

/*
 * Playlist container callbacks
 */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                           int position, void *userdata);
static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                             int position, void *userdata);
static void playlist_moved(sp_playlistcontainer *pc, sp_playlist *pl,
                             int position, int new_position, void *userdata);
static void container_loaded(sp_playlistcontainer *pc, void *userdata);

static sp_playlistcontainer_callbacks pc_callbacks = {
    .playlist_added = &playlist_added,
    .playlist_removed = &playlist_removed,
    .playlist_moved = &playlist_moved,
    .container_loaded = &container_loaded
};

/*
 * Playlist callbacks
 */
void tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata);
void tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata);
void tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata);
//void playlist_renamed(sp_playlist *pl, void *userdata);
//void playlist_state_changed(sp_playlist *pl, void *userdata);
//void playlist_update_in_progress(sp_playlist *pl, bool done, void *userdata);
//void playlist_metadata_updated(sp_playlist *pl, void *userdata);
//void track_created_changed(sp_playlist *pl, int position, sp_user *user, int when, void *userdata);
//void track_seen_changed(sp_playlist *pl, int position, bool seen, void *userdata);
//void description_changed(sp_playlist *pl, const char *desc, void *userdata);
//void image_changed(sp_playlist *pl, const byte *image, void *userdata);
//void track_message_changed(sp_playlist *pl, int position, const char *message, void *userdata);
//void subscribers_changed(sp_playlist *pl, void *userdata);

static sp_playlist_callbacks playlist_callbacks = {
    .tracks_added = &tracks_added,
    .tracks_removed = &tracks_removed,
    .tracks_moved = &tracks_moved
};


/*
 * Playlist container callback implementations
 */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata)
{
    sp_playlist_add_callbacks(pl, &playlist_callbacks, NULL);
}
static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata)
{
    sp_playlist_remove_callbacks(pl, &playlist_callbacks, NULL);
}
static void playlist_moved(sp_playlistcontainer *pc, sp_playlist *pl, int position, int new_position, void *userdata)
{
}
static void container_loaded(sp_playlistcontainer *pc, void *userdata)
{
    playlist_t *pl = (playlist_t *)userdata;
    pl->loaded = true;
}

/*
 * Playlist callback implementations
 */
void tracks_added(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata)
{
}
void tracks_removed(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
{
}
void tracks_moved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
{
}

playlist_t *playlist_init(sp_playlistcontainer *pc)
{
    playlist_t *pl = malloc(sizeof(playlist_t));
    // Get the playlist container
    pl->playlists = pc;
    pl->loaded = false;

    // Add callbacks
    sp_playlistcontainer_add_callbacks(pl->playlists, &pc_callbacks, pl);

    return pl;
}

/*
 * Helper function for serializing the playlist container
 */
json_t *parser_playlist(sp_playlist *pl)
{
    json_t *obj = json_object();

    json_object_set_new(obj, "type", json_string("playlist"));
    json_object_set_new(obj, "loaded", sp_playlist_is_loaded(pl) ? json_true() : json_false());
    json_object_set_new(obj, "name", json_string(sp_playlist_name(pl)));

    // Get the link
    sp_link *link = sp_link_create_from_playlist(pl);
    char buffer[200];
    buffer[199] = '\0';
    sp_link_as_string(link, buffer, 199);
    json_object_set_new(obj, "uri", json_string(buffer));

    return obj;
}

json_t *parser_folder(playlist_t *pl, uint32_t *i, uint32_t count)
{
    fprintf(stderr, "Parsing folder at %d\n", *i);
    // Create the folder object, and set some global attributes
    json_t *obj = json_object();

    // Set type
    json_object_set_new(obj, "type", json_string("folder"));

    // Get the folder name
    char folder_name[200];
    sp_playlistcontainer_playlist_folder_name(pl->playlists, *i, folder_name, 199);
    folder_name[199] = '\0';
    json_object_set_new(obj, "name", json_string(folder_name));

    // Playlist array
    json_t *children = json_array();

    // Get all the playlists in the folder
    for (*i+=1 ;*i < count; *i+=1) {
        sp_playlist_type type =  sp_playlistcontainer_playlist_type(pl->playlists, *i);
        if (type == SP_PLAYLIST_TYPE_PLAYLIST) {
            json_array_append(children, parser_playlist(sp_playlistcontainer_playlist(pl->playlists, *i)));
        } else if (type == SP_PLAYLIST_TYPE_START_FOLDER) {
            json_array_append(children, parser_folder(pl, i, count));
        } else if (type == SP_PLAYLIST_TYPE_END_FOLDER) {
            break;
        }
    }

    json_object_set_new(obj, "playlists", children);

    // Return the folder object
    return obj;
}

json_t *playlist_json(playlist_t *pl)
{
    if (!pl->loaded) return NULL;

    json_t *root = json_array();
    uint32_t count = sp_playlistcontainer_num_playlists(pl->playlists);
    for (uint32_t i = 0; i < count; i++) {
        sp_playlist_type type =  sp_playlistcontainer_playlist_type(pl->playlists, i);
        if (type == SP_PLAYLIST_TYPE_PLAYLIST) {
            json_array_append(root, parser_playlist(sp_playlistcontainer_playlist(pl->playlists, i)));
        } else if (type == SP_PLAYLIST_TYPE_START_FOLDER) {
            json_array_append(root, parser_folder(pl, &i, count));
        }
    }

    return root;
}
