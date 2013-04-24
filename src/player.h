#include <libspotify/api.h>

#include <stdbool.h>

#ifndef player_h
#define player_h

void *play_loop();
void init_player();
void flush_buffer();

extern int music_delivery(sp_session *sess, const sp_audioformat *format,
        const void *frames, int num_frames);

#endif
