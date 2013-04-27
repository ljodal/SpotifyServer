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

// Global variables for network

static void accept_connection(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx);
static void accept_error(struct evconnlistener *listener, void *ctx);

// Events
static void handle_read(struct bufferevent *bev, void *ctx);
static void handle_event(struct bufferevent *bev, short events, void *ctx);    

struct list
{
    struct bufferevent *bev;
    struct list *next;
    struct list *prev;
};

struct list *clients;

void add_client(struct bufferevent *bev)
{
    struct list *element = malloc(sizeof(struct list));
    element->bev = bev;
    element->next = clients;
    if (clients != NULL)
        clients->prev = element;
    element->prev = NULL;
    clients = element;
}

int remove_client(struct bufferevent *bev)
{
    struct list *tmp;

    for(tmp = clients; tmp != NULL; tmp = tmp->next)
    {
        if (bev == tmp->bev)
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

    bufferevent_setcb(bev, handle_read, NULL, handle_event, NULL);

    bufferevent_enable(bev, EV_READ | EV_WRITE);

    add_client(bev);
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
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *buf = bufferevent_get_input(bev);

    char *request_line;
    size_t len;

    request_line = evbuffer_readln(buf, &len, EVBUFFER_EOL_CRLF_STRICT);
    if (request_line) {
        char *response = handle_command(request_line);
        free(request_line);
        struct evbuffer *output = bufferevent_get_output(bev);
        evbuffer_add(output, response, strlen(response));
        evbuffer_add(output, "\r\n", 2);
        free(response);
    }
}

static void handle_event(struct bufferevent *bev, short events, void *ctx)
{
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        fprintf(stderr, "Disconnected\n");
        if (remove_client(bev) != 0)
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
        struct evbuffer *output = bufferevent_get_output(tmp->bev);
        evbuffer_add(output, data, len);
        evbuffer_add(output, "\r\n", 2);
    }
}

void send_msg(uint8_t *data, size_t len, struct bufferevent *bev)
{
    return;
}
