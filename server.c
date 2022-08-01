#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "func.h"
#include "game.h"

#define LISTEN_BACKLOG 64

#define ERR_EXIT(s) perror(s); exit(1);

int init_server(int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        ERR_EXIT("socket");
    }
    struct sockaddr_in socket_addr;
    memset(&socket_addr, 0, sizeof(struct sockaddr_in));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) == -1) {
        ERR_EXIT("bind");
    }
    if (listen(socket_fd, LISTEN_BACKLOG) == -1) {
        ERR_EXIT("listen");
    }
    return socket_fd;   
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s port_number\n", argv[0]);
        exit(1);
    }
    int port;
    if (!str_to_num(argv[1], &port)) {
        ERR_EXIT("port number should be integer");
    }
    int socket_fd = init_server(port);
    int player_fd[PLAYER_NUM];
    int player_count = 0;
    printf("start loading resource\n");
    load_resource();
    printf("finish loading resource\n");
    while (1) {
        int connect_fd = accept(socket_fd, NULL, NULL);
        if (connect_fd < 0) {
            ERR_EXIT("accept");
        }
        player_fd[player_count++] = connect_fd;
        printf("pending player: %d\n", player_count);
        show_icon(connect_fd);
        for (int i = 0; i < player_count; i++) {
            write_waiting_message(player_fd[i], player_count);
        }
        if (player_count < PLAYER_NUM) {
            continue;
        }
        player_count = 0;
        int child_pid = fork();
        if (child_pid < 0) {
            ERR_EXIT("fork");
        }
        if (child_pid) {
            // parent
            printf("game starts (pid: %d)\n", child_pid);
            for (int i = 0; i < PLAYER_NUM; i++) {
                close(player_fd[i]);
            }
            continue;
        }
        // child
        close(socket_fd);
        run_game(player_fd);
        return 0;
    }
    return 0;
}
