#include "commands.h"

#include <string.h>

#include "spotify.h"
#include "server.h"

// TODO Don't use strtok

struct callback
{
    struct bufferevent *bev;
    json_t *user_data;
};

char *handle_command(char *json, struct bufferevent *bev)
{
    json_error_t error;

    // Reponse object
    json_t *response = NULL;

    // Parse the JSON command
    json_t *command = json_loads(json, 0, &error);

    if (command) {
        json_t *c = json_object_get(command, "command");
        if (!c) return NULL;

        const char *cmd = json_string_value(c);

        if (!cmd) {
            return NULL;
        } else if (strcmp(cmd, "queue_add") == 0) {
            json_t *t = json_object_get(command, "type");

            if (json_typeof(t) != JSON_STRING) {
                return NULL;
            } else if (strcmp(json_string_value(t), "uri") == 0) {

                json_t *uri = json_object_get(command,"uri");

                // Queue the uri
                queue_link((char *)json_string_value(uri));
            }
        } else if (strcmp(cmd, "queue") == 0) {
            queue_broadcast();
        } else if (strcmp(cmd, "queue_delete") == 0) {
            uint32_t index = json_integer_value(json_object_get(command, "index"));
            queue_delete(index);
        } else if (strcmp(cmd, "queue_move") == 0) {
            uint32_t from = json_integer_value(json_object_get(command, "from"));
            uint32_t to = json_integer_value(json_object_get(command, "to"));
            queue_move(from, to);
        } else if (strcmp(cmd, "search") == 0) {
            json_t *query = json_object_get(command, "query");


            if (query && json_typeof(query) == JSON_STRING) {
                // Create a callback struct
                callback_t *cb = malloc(sizeof(callback_t));
                cb->bev = bev;
                json_t *user_data = json_object_get(command, "user_data");
                if (user_data) {
                    json_incref(user_data);
                    cb->user_data = user_data;
                }

                // Start the search command
                search(json_string_value(query), cb);
            }
        } else {
            fprintf(stderr, "Unknown\n");
        }
    } else {
        response = json_object();

        json_object_set_new(response, "success", json_false());
        json_object_set_new(response, "type", json_string("invalid_json"));
        json_object_set_new(response, "message", json_string("Invalid JSON command"));
    }

    if (response) {
        char *retval = json_dumps(response, JSON_COMPACT);
        json_decref(response);
        return retval;
    } else {
        return NULL;
    }
}

void send_callback(callback_t *cb, json_t *data)
{
    json_object_set_new(data, "user_data", cb->user_data);

    char *msg = json_dumps(data, JSON_COMPACT);
    send_msg(msg, strlen(msg), cb->bev);
    free(msg);
}
