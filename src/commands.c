#include "commands.h"

#include <jansson.h>
#include <string.h>

#include "spotify.h"

// TODO Don't use strtok

char *handle_command(char *cmd)
{
    json_t *json = json_object();

    if (!strncmp(cmd, "play_next", 9)) {
        play_next();
        json_object_set_new(json, "type", json_string("play_next"));
        json_object_set_new(json, "success", json_true());
        json_object_set_new(json, "message", json_string("Next song will be played."));
    } else if (!strncmp(cmd, "play_prev", 9)) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("play_prev"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
    } else if (!strncmp(cmd, "play", 4)) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("play"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
    } else if (!strncmp(cmd, "queue_uri", 9)) {
        char *uri = NULL;

        // Find the uri in the string, it's specified
        if (strlen(cmd) > 10) {
            uri = strchr(cmd, ' ')+1;
        }
        if (!uri) {
            json_object_set_new(json, "type", json_string("queue_uri"));
            json_object_set_new(json, "success", json_false());
            json_object_set_new(json, "message", json_string("No URI given."));
        } else {
            spotify_status_t ret = queue_link(uri);
            if (ret == SPOTIFY_TRACK_QUEUED) {
                json_object_set_new(json, "type", json_string("queue_uri"));
                json_object_set_new(json, "success", json_true());
                json_object_set_new(json, "message", json_string("Track added to the queue."));
            } else if (ret == SPOTIFY_ALBUM_QUEUED) {
                json_object_set_new(json, "type", json_string("queue_uri"));
                json_object_set_new(json, "success", json_true());
                json_object_set_new(json, "message", json_string("Album added to the queue."));
            } else if (ret == SPOTIFY_INVALID_URI) {
                json_object_set_new(json, "type", json_string("queue_uri"));
                json_object_set_new(json, "success", json_false());
                json_object_set_new(json, "message", json_string("Invalid URI."));
            } else {
                json_object_set_new(json, "type", json_string("queue_uri"));
                json_object_set_new(json, "success", json_false());
                json_object_set_new(json, "message", json_string("Unknown error occured."));
            }
        }
    } else if (!strncmp(cmd, "queue_delete", 12)) {
        uint32_t from = 0;

        // Get the index
        if (sscanf(cmd, "queue_delete %u", &from) == 1) {
            queue_delete(from);
        } else {
            json_object_set_new(json, "type", json_string("queue_delete"));
            json_object_set_new(json, "success", json_false());
            json_object_set_new(json, "message", json_string("Invalid arguments"));
        }
    } else if (!strncmp(cmd, "queue_move", 10)) {
        uint32_t from = 0, to = 0;

        // Get the indexes
        if (sscanf(cmd, "queue_move %u %u", &from, &to) == 2) {
            queue_move(from, to);
        } else {
            json_object_set_new(json, "type", json_string("queue_move"));
            json_object_set_new(json, "success", json_false());
            json_object_set_new(json, "message", json_string("Invalid arguments"));
        }
    } else if (!strncmp(cmd, "queue", 5)) {
        queue_broadcast();
        json_object_set_new(json, "type", json_string("queue"));
        json_object_set_new(json, "success", json_true());
        json_object_set_new(json, "message", json_string("Queue broadcasted"));
    } else if (!strncmp(cmd, "stop", 4)) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("stop"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
    } else if (!strncmp(cmd, "search", 6)) {
        char *query = NULL;

        if (strlen(cmd) > 7) {
            query = strchr(cmd, ' ');
        }

        search(query, NULL);
        json_object_set_new(json, "type", json_string("search"));
        json_object_set_new(json, "success", json_true());
        json_object_set_new(json, "message", json_string("Searching …"));
    } else {
        // TODO Unknown command, reply
        json_object_set_new(json, "type", json_string(cmd));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Unknown command."));
    }

    char *retval = json_dumps(json, JSON_COMPACT);
    json_decref(json);
    return retval;
}
