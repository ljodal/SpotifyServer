#ifndef server_h
#define server_h

#include <inttypes.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

// Server functions
void init_server(int port);
void run_server();
void destroy_server();
void broadcast(char *data, size_t len);
void send_msg(uint8_t *data, size_t len, struct bufferevent *bev);

// Varibales
struct event_base *base;
struct evconnlistener *listener;

#endif
