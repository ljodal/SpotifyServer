#ifndef commands_h
#define commands_h

#include <event2/bufferevent.h>
#include <jansson.h>

typedef struct callback callback_t;

char *handle_command(char *command, struct bufferevent *bev);
void send_callback(callback_t *cb, json_t *data);

#endif
