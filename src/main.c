#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>

#include <pthread.h>

#include "server.h"
#include "spotify.h"
#include "player.h"

void exit_program();

const char *username = NULL;
char *password = NULL;

void print_usage()
{
    fprintf(stderr, "Usage: -u <username> [-p <port>]\n");
}

int main(int argc, char *argv[])
{
    int port = 0, opt;
    while ((opt = getopt(argc, argv, "hu:p:")) != EOF) {
        switch (opt) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);

            case 'u':
                username = optarg;
                break;

            case 'P':
                port = atoi(optarg);
                break;

            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    // Get the password
    password = getpass("Password: ");

    if (!username || !password)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, exit_program);

    pthread_t player_thread;

    if (port == 0)
        port = 3579;

    int err = 0;
    //err = pthread_create(&server_thread, NULL, run_server, &port);
    //if (err != 0) {
    //    fprintf(stderr, "Error creating network thread.\n");
    //    return EXIT_FAILURE;
    //}
    //

    init_server(port);

    init_spotify(username, password);

    // Free password
    free(password);

    err = pthread_create(&player_thread, NULL, play_loop, NULL);
    if (err != 0) {
        fprintf(stderr, "Error creating the player thread.\n");
        return EXIT_FAILURE;
    }

    //spotify_loop();

    run_server();

    return EXIT_SUCCESS;

}

void exit_program()
{
    destroy_server();
    exit(EXIT_SUCCESS);
}
