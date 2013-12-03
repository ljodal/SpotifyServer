#ifndef client_h
#define client_h

#include <event2/bufferevent.h>
#include <libspotify/api.h>

typedef struct client client_t;

client_t *init_client(struct bufferevent *bev);
void client_free(client_t *c);
void client_playlist_on(client_t *c, sp_playlist *pl);
void client_playlist_off(client_t *c, sp_playlist *pl);
void client_send(client_t* c, char *data, size_t len);

#endif
