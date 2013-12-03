#ifndef commands_h
#define commands_h

#include <event2/bufferevent.h>
#include <jansson.h>

#include "client.h"

typedef struct callback callback_t;

void handle_command(char *command, client_t *client);
void send_callback(callback_t *cb, json_t *data);

#endif
