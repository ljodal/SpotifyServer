#ifndef commands_h
#define commands_h

#include <event2/bufferevent.h>

char *handle_command(char *command, struct bufferevent *bev);

#endif
