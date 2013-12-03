#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include <libspotify/api.h>
#include <jansson.h>

#include "spotify.h"
#include "player.h"
#include "playqueue.h"
#include "server.h"
#include "commands.h"
#include "key.h"
#include "playlist.h"

#define QUEUE_SIZE 2

static sp_session *sess;
static playlist_t *pl;

/*
 * Session callbacks
 */
static void logged_in(sp_session *sp, sp_error error);
static void notify_main_thread(sp_session *sp);
static void metadata_updated(sp_session *sp);
static void play_token_lost(sp_session *sp);
static void end_of_track(sp_session *sp);

static sp_session_callbacks session_callbacks = {
    .logged_in = &logged_in,
    .notify_main_thread = &notify_main_thread,
    .music_delivery = &music_delivery,
    .metadata_updated = &metadata_updated,
    .play_token_lost = &play_token_lost,
    .log_message = NULL,
    .end_of_track = &end_of_track,
};

/*
 * Spotify config
 */
static sp_session_config spconfig = {
    .api_version = SPOTIFY_API_VERSION,
    .cache_location = "tmp",
    .settings_location = "~/.config",
    .application_key = g_appkey,
    .application_key_size = sizeof(g_appkey), // Set in key.c
    .user_agent = "network-player-test",
    .callbacks = &session_callbacks,
    NULL,
};

/*
 * Broadcast status
 */
static void status_update(evutil_socket_t fd, short events, void *arg);

/*********************************************************
 *
 * Handles the queue
 *
 * *******************************************************/

playqueue_t *pq;

/*
 * This function adds a track to the play queue
 */
static int queue_track(sp_link *link)
{
    sp_track *track = sp_link_as_track(link);

    if (!track)
        return SPOTIFY_TRACK_NOT_QUEUED;

    if(playqueue_add(pq, track) == 0) {
        queue_broadcast();
        return SPOTIFY_TRACK_QUEUED;
    }

    return SPOTIFY_ERROR;
}

/*
 * These functions adds an entire album to the play queue
 */
static void queue_album_callback(sp_albumbrowse *browse, void *userdata)
{
    json_t *json = json_object();
    json_object_set_new(json, "type", json_string("queue_album"));

    if (sp_albumbrowse_error(browse) != SP_ERROR_OK) {
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Unable to add album to queue."));
        char *data = json_dumps(json, JSON_COMPACT);
        //broadcast(data, strlen(data));
        json_decref(json);
        free(data);
        return;
    }

    int tracks = sp_albumbrowse_num_tracks(browse);
    for (int track = 0; track < tracks; track++) {
        playqueue_add(pq, sp_albumbrowse_track(browse, track));
    }
    sp_albumbrowse_release(browse);

    json_object_set_new(json, "success", json_true());
    json_object_set_new(json, "message", json_string("Album added to queue."));
    char *data = json_dumps(json, JSON_COMPACT);
    //broadcast(data, strlen(data));
    json_decref(json);
    free(data);

    queue_broadcast();
}

static int queue_album(sp_link *link)
{
    sp_album *album = sp_link_as_album(link);
    if (!album)
        return SPOTIFY_ALBUM_NOT_QUEUED;

    sp_albumbrowse_create(sess, album, queue_album_callback, NULL);

    return SPOTIFY_ALBUM_QUEUED;
}

/*
 * This function adds an entire playlist to the play queue
 */
static int queue_playlist(sp_link *link)
{
    return SPOTIFY_ERROR;

    sp_playlist *playlist = sp_playlist_create(sess, link);

    if (sp_playlist_is_loaded(playlist)) {
        sp_playlist_release(playlist);
        return SPOTIFY_PLAYLIST_QUEUED;
    } else {
        return SPOTIFY_PLAYLIST_NOT_QUEUED;
    }
}

/*
 * This function adds one or more songs from a link to the queue.
 *
 * The link can be a link to an playlist, an album or a song.
 */
int queue_link(char *link)
{
    spotify_status_t retval = SPOTIFY_ERROR;
    sp_link *l = sp_link_create_from_string(link);
    switch(sp_link_type(l))
    {
        case SP_LINKTYPE_ALBUM: retval = queue_album(l);
                                break;
        case SP_LINKTYPE_PLAYLIST: retval = queue_playlist(l);
                                   break;
        case SP_LINKTYPE_TRACK: retval = queue_track(l);
                                break;
        case SP_LINKTYPE_INVALID: retval = SPOTIFY_INVALID_URI;
                                  break;
        default: retval = SPOTIFY_ERROR;
    }
    sp_link_release(l);

    notify_main_thread(sess);

    return retval;
}

/**
 * Manipulate the queue
 */

// Broadcast all elements in the queue
void queue_broadcast() {
    json_t *json = json_object();

    json_t *tracks = json_array();
    playqueue_to_json(pq, tracks);

    json_object_set_new(json, "type", json_string("queue"));
    json_object_set_new(json, "tracks", tracks);

    char *data = json_dumps(json, JSON_COMPACT);
    broadcast(data, strlen(data));
    json_decref(json);
    free(data);
}

// Move the queue element 'from' to 'to'
void queue_move(uint32_t from, uint32_t to) {
    playqueue_move(pq, from, to);
    queue_broadcast();
}

// Delete the element at index 'i' in the queue.
void queue_delete(uint32_t i) {
    playqueue_delete(pq, i);
    queue_broadcast();
}

/*********************************************************
 *
 * Handles the playback
 *
 * *******************************************************/

static sp_track *currenttrack;
bool _playing;
struct event *status_event;

/*
 * This function tries to start playback
 */
int try_playback(sp_session *sp)
{
    if (!currenttrack) {
        sp_track *track = playqueue_next(pq);
        // If no track in queue, return with error
        if (!track)
            return -1;

        // Set track as the current
        currenttrack = track;
    }

    if (sp_track_error(currenttrack) != SP_ERROR_OK) {
        return -3;
    }

    queue_broadcast();

    json_t *json = json_object();
    json_object_set_new(json, "type", json_string("now_playing"));
    json_object_set_new(json, "success", json_true());
    json_object_set_new(json, "message", json_string("Track changed."));
    json_object_set_new(json, "track_name", json_string(sp_track_name(currenttrack)));
    char *data = json_dumps(json, JSON_COMPACT);
    json_decref(json);
    broadcast(data, strlen(data));
    free(data);


    sp_session_player_load(sp, currenttrack);
    sp_session_player_play(sp, 1);

    _playing = true;

    return 0;
}

/*
 * Initializer
 */
void init_spotify(const char *username, const char *password)
{
    // Initialize playing state as false
    _playing = false;

    // Create a temporary session pointer
    sp_session *sp;

    // Check that both username and password is pressent
    if (!username || !password)
    {
        fprintf(stderr, "Username or password missing.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the play queue
    pq = playqueue_init(QUEUE_SIZE);
    if (!pq)
    {
        fprintf(stderr, "Unable to create play queue.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the player
    init_player();

    /* Create session */
    int err = sp_session_create(&spconfig, &sp);
    if (SP_ERROR_OK != err) {
        fprintf(stderr, "Unable to create session: %s\n",
                sp_error_message(err));
        exit(EXIT_FAILURE);
    }

    // Prefer high bitrate
    sp_session_preferred_bitrate(sp, SP_BITRATE_320k);

    // Initialize current track to NULL
    currenttrack = NULL;

    // Set the global session pointer
    sess = sp;

    // Try to create the update event
    /*
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    status_event = event_new(base, -1, EV_PERSIST, status_update, NULL);
    event_add(status_event, &tv);
    */

    // Log in
    sp_session_login(sp, username, password, 0, NULL);
#ifdef DEBUG
    fprintf(stderr, "Spotify initialized!\n");
#endif
}

/*
 * The loop where all spotify event are handled
 */
static void process_events(evutil_socket_t fd, short events, void *arg)
{
    if (!_playing)
        try_playback(sess);

    int next_timeout = 0;
    do {
        sp_error err = sp_session_process_events(sess, &next_timeout);
        if (err != SP_ERROR_OK) {
            fprintf(stderr, "Err: %d\n", err);
            exit(EXIT_FAILURE);
        }
    } while (next_timeout == 0);

    //fprintf(stderr, "Next timeout %d\n", next_timeout);
    struct timeval tv = {next_timeout / 1000, (next_timeout % 1000) * 1000};
    event_base_once(base, -1, EV_TIMEOUT, process_events, NULL, &tv);
}

static void notify_main_thread(sp_session *sp)
{
    event_base_once(base, -1, EV_TIMEOUT, process_events, NULL, NULL);
}

static void play_token_lost(sp_session *sp)
{
    fprintf(stderr, "Token lost!\n");
    return;
}

static void end_of_track(sp_session *sp)
{
    fprintf(stderr, "End of track!\n");

    if (currenttrack) {
        sp_track_release(currenttrack);
        currenttrack = NULL;
        _playing = false;
        sp_session_player_unload(sp);
    }

    notify_main_thread(sp);
}

static void logged_in(sp_session *sp, sp_error error)
{
    if (SP_ERROR_OK != error) {
        fprintf(stderr, "Login failed: %s\n",
                sp_error_message(error));
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Logged in\n");

    sp_playlistcontainer *pc = sp_session_playlistcontainer(sp);
    pl = playlist_init(pc);

    notify_main_thread(sp);
}

/**
 * Callback called when libspotify has new metadata available
 *
 * @sa sp_session_callbacks#metadata_updated
 */
static void metadata_updated(sp_session *sp)
{
    notify_main_thread(sp);
}

/*
 * Handle search
 */

static void search_complete(sp_search *search, void *userdata)
{
    json_t *json = json_object();
    json_object_set_new(json, "type", json_string("search"));
    json_object_set_new(json, "success", json_true());


    // Go through all albums and add them to the result
    // TODO Error checking
    int album_count = sp_search_num_albums(search);
    json_t *albums = json_array();
    for (int i = 0; i < album_count; i++) {
        sp_album *album = sp_search_album(search, i);

        json_t *album_obj = json_object();
        json_object_set_new(album_obj, "name", json_string(sp_album_name(album)));
        json_object_set_new(album_obj, "artist", json_string(sp_artist_name(sp_album_artist(album))));

        // Get the uri of the album
        sp_link *link = sp_link_create_from_album(album);
        char buf[256];
        sp_link_as_string(link, buf, 256);
        json_object_set_new(album_obj, "uri", json_string(buf));
        sp_link_release(link);

        json_array_append(albums, album_obj);
        json_decref(album_obj);
    }

    json_object_set(json, "albums", albums);
    json_decref(albums);

    // Go through all the tracks
    int track_count = sp_search_num_tracks(search);
    json_t *tracks = json_array();
    for (int i = 0; i < track_count; i++) {
        sp_track *track = sp_search_track(search, i);
        json_t *track_obj = json_object();
        json_object_set_new(track_obj, "name", json_string(sp_track_name(track)));
        json_object_set_new(track_obj, "artist", json_string(sp_artist_name(sp_track_artist(track, 0))));
        json_object_set_new(track_obj, "album", json_string(sp_album_name(sp_track_album(track))));
        json_object_set_new(track_obj, "duration", json_integer(sp_track_duration(track)));

        // Get the uri of the album
        sp_link *link = sp_link_create_from_track(track, 0);
        char buf[256];
        sp_link_as_string(link, buf, 256);
        json_object_set_new(track_obj, "uri", json_string(buf));
        sp_link_release(link);

        json_array_append(tracks, track_obj);
        json_decref(track_obj);
    }

    json_object_set(json, "tracks", tracks);
    json_decref(tracks);

    // Go through all the artists
    int artist_count = sp_search_num_artists(search);
    json_t *artists = json_array();
    for (int i = 0; i < artist_count; i++) {
        sp_artist *artist = sp_search_artist(search, i);
        json_t *artist_obj = json_object();
        json_object_set_new(artist_obj, "name", json_string(sp_artist_name(artist)));

        // Get the uri of the album
        sp_link *link = sp_link_create_from_artist(artist);
        char buf[256];
        sp_link_as_string(link, buf, 256);
        json_object_set_new(artist_obj, "uri", json_string(buf));
        sp_link_release(link);

        json_array_append(artists, artist_obj);
        json_decref(artist_obj);
    }

    json_object_set(json, "artists", artists);
    json_decref(artists);

    if (userdata != NULL) {
        send_callback((callback_t *)userdata, json);
    } else {
        char *data = json_dumps(json, JSON_COMPACT);
        broadcast(data, strlen(data));
        free(data);
    }
    json_decref(json);

    // Do something
    sp_search_release(search);
}

int search(const char *query, void *searcher)
{
    fprintf(stderr, "Searching for %s\n", query);
    sp_search_create(
            sess,
            query,
            0,
            100,
            0,
            100,
            0,
            100,
            0,
            0,
            SP_SEARCH_STANDARD,
            search_complete,
            searcher);
    return -1;
}

/*
 * Handle play next
 */
void play_next()
{
    if (currenttrack) {
        sp_track_release(currenttrack);
        currenttrack = NULL;
        _playing = false;
        sp_session_player_unload(sess);
    }

    flush_buffer();

    notify_main_thread(sess);
}

void playlists(void *cb)
{
    if (pl) {
        json_t *obj = json_object();
        json_object_set_new(obj, "type", json_string_nocheck("playlists"));
        json_object_set_new(obj, "playlists", playlist_json(pl));

        send_callback(cb, obj);

        json_decref(obj);
    } else {
        fprintf(stderr, "Playlist not initialized\n");
    }
}

void playlist_start(char *uri, client_t *c)
{
    sp_link *link = sp_link_create_from_string(uri);
    sp_playlist *playlist = sp_playlist_create(sess, link);

    client_playlist_on(c, playlist);
}

void playlist_stop(char *uri, client_t *c)
{
    sp_link *link = sp_link_create_from_string(uri);
    sp_playlist *playlist = sp_playlist_create(sess, link);

    client_playlist_off(c, playlist);
}

/*********************************************************
 *
 * Send out status updates
 *
 * *******************************************************/
static void status_update(evutil_socket_t fd, short events, void *arg)
{
    json_t *json = json_object();

    if (currenttrack != NULL) {
        json_object_set_new(json, "playing", json_true());
    } else {
        json_object_set_new(json, "playing", json_false());
    }

    fprintf(stderr, "%s\n", json_dumps(json, JSON_COMPACT));
    json_decref(json);
}
