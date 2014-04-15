#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

// Network includes
#include <sys/types.h>
#include <sys/socket.h>

// libevent includes
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>

// Threads
#include <pthread.h>

#include "server.h"

#include "commands.h"
#include "client.h"

// Global variables for network

static void accept_connection(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
static void accept_error(struct evconnlistener *listener, void *ctx);

// Events
static void handle_read(struct bufferevent *bev, void *ctx);
static void handle_event(struct bufferevent *bev, short events, void *ctx);

struct list
{
    client_t *c;
    struct list *next;
    struct list *prev;
};

struct list *clients;

void add_client(client_t *c)
{
    struct list *element = malloc(sizeof(struct list));
    element->c = c;
    element->next = clients;
    if (clients != NULL)
        clients->prev = element;
    element->prev = NULL;
    clients = element;
}

int remove_client(client_t *c)
{
    struct list *tmp;

    for(tmp = clients; tmp != NULL; tmp = tmp->next)
    {
        if (c == tmp->c)
        {
            if(tmp->prev != NULL) {
                tmp->prev->next = tmp->next;
            } else {
                clients = tmp->next;
            }
            free(tmp);
            return 0;
        }
    }
    return -1;
}


void init_server(int port) {
    fprintf(stderr, "Using port %d\n", port);

    clients = NULL;

    // Make it thread safe
    int err = evthread_use_pthreads();
    if (err != 0)
    {
        fprintf(stderr, "Unable to make libevent thread safe");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sin;

    // Create a new event base
    base = event_base_new();
    if (!base)
    {
        fprintf(stderr, "Unable to open event base\n");
        exit(EXIT_FAILURE);
    }

    // Set up the socket
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    // Create a new listener
    listener = evconnlistener_new_bind(base, accept_connection, NULL,
            LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
            (struct sockaddr *) &sin, sizeof(sin));
    if (!listener)
    {
        perror("Unable to create listener");
        exit(EXIT_FAILURE);
    }

    evconnlistener_set_error_cb(listener, accept_error);
}

void run_server() {
#ifdef DEBUG
    fprintf(stderr, "Server ready to run.\n");
#endif
    // Run the event loop
    event_base_dispatch(base);
}

// Free allocated stuff
void destroy_server() {
    event_base_free(base);
    evconnlistener_free(listener);
}

static void accept_connection(struct evconnlistener *listener,
        evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx)
{
    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
        fprintf(stderr, "Unable to create bufferevent\n");

    // Create a new client
    client_t *c = init_client(bev);

    // Set up the buffer
    bufferevent_setcb(bev, handle_read, NULL, handle_event, c);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    // Store the client
    add_client(c);
}

static void accept_error(struct evconnlistener *listener, void *ctx)
{
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}

static void handle_read(struct bufferevent *bev, void *ctx)
{
    // Get the buffer
    struct evbuffer *buf = bufferevent_get_input(bev);

    // Get the client
    client_t *c = ctx;

    // Get the message
    char *request_line;
    size_t len;

    // Read all lines if more than 1 have arrived
    for (;;) {
        request_line = evbuffer_readln(buf, &len, EVBUFFER_EOL_CRLF_STRICT);

        // If we got a messsage, handle it
        if (request_line) {
            handle_command(request_line, c);
            free(request_line);
        } else {
            return;
        }
    }
}

static void handle_event(struct bufferevent *bev, short events, void *ctx)
{
    client_t *c = ctx;
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        fprintf(stderr, "Disconnected\n");
        if (remove_client(c) != 0)
        {
            fprintf(stderr, "Unable to remove client.\n");
        } else {
            fprintf(stderr, "Client removed!\n");
        }
        bufferevent_free(bev);
    }
}

void broadcast(char *data, size_t len) {
    struct list *tmp;
    for(tmp = clients; tmp; tmp = tmp->next) {
        client_send(tmp->c, data, len);
    }
}
