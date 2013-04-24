#include "commands.h"

#include <jansson.h>
#include <string.h>

#include "spotify.h"

char *handle_command(char *command)
{
    json_t *json = json_object();

    char *cmd = strtok(command, " ");
    if (!strcmp(cmd, "play")) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("play"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
    } else if (!strcmp(cmd, "play_next")) {
        play_next();
        json_object_set_new(json, "type", json_string("play_next"));
        json_object_set_new(json, "success", json_true());
        json_object_set_new(json, "message", json_string("Next song will be played."));
    } else if (!strcmp(cmd, "play_prev")) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("play_prev"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
    } else if (!strcmp(cmd, "queue_uri")) {
        char *uri = strtok(NULL, " ");
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
    } else if (!strcmp(cmd, "stop")) {
        // TODO Not implemented, do nothing
        json_object_set_new(json, "type", json_string("stop"));
        json_object_set_new(json, "success", json_false());
        json_object_set_new(json, "message", json_string("Not implemented"));
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
