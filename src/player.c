#include <pthread.h>

#include <ao/ao.h>

#include "player.h"
#include "queue.h"
#include "spotify.h"

#include <string.h>

#define BUFSIZE 8192
#define BUFNUM 8

pthread_mutex_t buffer_lock;
pthread_cond_t play_wait;
struct queue *queue;

struct buffer
{
    void *data;
    int size;
};

#ifndef pi
static ao_device* ao_dev = NULL;
int ao_d;
int frame_size;
ao_sample_format ao_format;
#else
int samplerate = 0;
int bitdepth = 0;
int channels = 0;
char *output = "hdmi"; // Change to "local" if you want analog output
AUDIOPLAY_STATE_T *st = NULL;
#endif

void init_player()
{
    if (pthread_mutex_init(&buffer_lock, NULL) != 0)
    {
        fprintf(stderr, "Unable to initialize buffer mutex.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&play_wait, NULL) != 0)
    {
        fprintf(stderr, "Unable to initalize play condition.\n");
        exit(EXIT_FAILURE);
    }

    queue = queue_init();
    if (!queue)
    {
        fprintf(stderr, "Unable to initialize buffer queue.\n");
        exit(EXIT_FAILURE);
    }

    // Allocate and add elements to the queue
    for(int i = 0; i < BUFNUM; i++) {
        struct buffer *buf = calloc(1, sizeof(struct buffer));
        buf->data = malloc(BUFSIZE);
        queue_add_free(queue, buf);
    }

#ifndef pi
    // Initialize libao
    ao_initialize();

    ao_d = ao_default_driver_id();
    if (ao_d < 0)
    {
        fprintf(stderr, "Unable to initialize audio output dirver.\n");
        exit(EXIT_FAILURE);
    }
#endif

#ifdef DEBUG
    fprintf(stderr, "Player initialized.\n");
#endif
}

void *play_loop()
{
    pthread_mutex_lock(&buffer_lock);

    for(;;)
    {
        struct buffer *buf = queue_pop(queue);

        if (!buf || buf->size <= 0)
        {
            // Wait for new buffers to become available
            pthread_cond_wait(&play_wait, &buffer_lock);
        } else {
            pthread_mutex_unlock(&buffer_lock);

            if(!ao_play(ao_dev, buf->data, buf->size))
                fprintf(stderr, "Unable to play buffer\n");

            pthread_mutex_lock(&buffer_lock);
        }

        queue_add_free(queue, buf);
    }
}

void format_change(const sp_audioformat* format)
{
#ifndef pi
    if (ao_dev)
        ao_close(ao_dev);
#else
    if (st)
        audioplay_delete(st);
#endif

#ifndef pi
    // Set all parameters of the format struct
    ao_format.bits = 16;
    ao_format.rate = format->sample_rate;
    ao_format.channels = format->channels;
    ao_format.byte_format = AO_FMT_NATIVE;
    ao_format.matrix = NULL;
#else
    bitdepth = 16;
    samplerate = format->sample_rate;
    channels = format->channels;
#endif

    // Update the frame size variable
    frame_size = sizeof(int16_t) * format->channels;

#ifndef pi
    // Open an AO device
    ao_dev = ao_open_live(ao_d, &ao_format, NULL);
    if (!ao_dev)
    {
        fprintf(stderr, "Unable to open AO device.\n");
        exit(EXIT_FAILURE);
    }
#else
    // Last to parameters are number of buffers and the size of the buffers
    int err = audioplay_create(&st, samplerate, channels, bitdepth, BUFNUM, (BUFSIZE * frame_size) >> 3);
    if (err != 0) {
        fprintf(stderr, "Unable to create audioplay\n");
        exit(EXIT_FAILURE);
    }

    // Set the output device
    err = audioplay_set_dest(st, output);
    if (err != 0) {
        fprintf(stderr, "Unable to create audioplay\n");
        exit(EXIT_FAILURE);
    }
#endif

#ifdef DEBUG
    fprintf(stderr, "Player changed format.\n");
#endif
}

void flush_buffer()
{
    pthread_mutex_lock(&buffer_lock);

    struct buffer *buf = queue_pop(queue);
    while (buf) {
        queue_add_free(queue, buf);
        buf = queue_pop(queue);
    }

    pthread_mutex_unlock(&buffer_lock);
}

extern int music_delivery(sp_session *sess, const sp_audioformat *format,
        const void *frames, int num_frames)
{
    if (num_frames == 0) {
        flush_buffer();
        return 0;
    } else {

        pthread_mutex_lock(&buffer_lock);

        // If not already done, prepare for playing
        if (!ao_dev)
            format_change(format);

        // The format changed
        if (format->sample_rate != ao_format.rate || format->channels != ao_format.channels) {

            // Wait untill the queue is empty
            if (queue_length(queue) != 0) {
                pthread_mutex_unlock(&buffer_lock);
                return 0;
            } else {
                format_change(format);
            }
        }

        // Figure out how much data to store
        int size = BUFSIZE < num_frames * frame_size ? BUFSIZE : num_frames * frame_size;

        // Only store hole frames
        size = size - (size % frame_size);

        // Add the new data
        struct buffer *buf = queue_get_free(queue);
        if (buf) {
            memcpy(buf->data, frames, size);
            buf->size = size;
            queue_push(queue, buf);
        } else {
            size = 0;
        }

        // We have new data!
        pthread_cond_signal(&play_wait);

        pthread_mutex_unlock(&buffer_lock);

        return size / frame_size;
    }
}
