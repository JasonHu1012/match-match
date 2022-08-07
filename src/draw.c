#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "func.h"

#define CARD_HEIGHT 28
#define CARD_WIDTH 38

#define BUFFER_HEIGHT 64
#define BUFFER_WIDTH 64
#define CARD_EXTENSION ".card"

bool exist_file(char *path) {
    return access(path, F_OK) == 0;
}

// check whether arguments are valid
void arg_valid(int argc, char **argv) {
    if (BUFFER_HEIGHT < CARD_HEIGHT || BUFFER_WIDTH < CARD_WIDTH) {
        fprintf(stderr, "error: buffer smaller than card\n");
        exit(1);
    }
    if (argc != 2) {
        fprintf(stderr, "usage: %s [output file]\n", argv[0]);
        exit(1);
    }
}

// read content from stdin
void read_content(char **card, char **buffer) {
    int height = 0;
    while (height < BUFFER_HEIGHT && fgets(buffer[height], BUFFER_WIDTH + 2, stdin)) {
        int len = strlen(buffer[height]);
        if (buffer[height][len - 1] == '\n') {
            buffer[height][len - 1] = ' ';
        }
        else {
            char test_new_line = getchar();
            if (test_new_line != '\n') {
                fprintf(stderr, "error: content too wide\n");
                exit(1);
            }
        }
        buffer[height++][len] = ' ';
    }
    if (height == BUFFER_HEIGHT) {
        char test_eof[2];
        if (fgets(test_eof, 2, stdin)) {
            fprintf(stderr, "error: content too high\n");
            exit(1);
        }
    }
}

// locate the content
void locate(char **buffer, int *top, int *bottom, int *left, int *right) {
    bool all_space;
    all_space = true;
    *top = -1;
    do {
        (*top)++;
        if (*top == BUFFER_HEIGHT) {
            fprintf(stderr, "error: no content\n");
            exit(1);
        }
        for (int i = 0; i < BUFFER_WIDTH; i++) {
            if (buffer[*top][i] != ' ') {
                all_space = false;
                break;
            }
        }
    } while (all_space);
    all_space = true;
    *bottom = BUFFER_HEIGHT;
    do {
        (*bottom)--;
        for (int i = 0; i < BUFFER_WIDTH; i++) {
            if (buffer[*bottom][i] != ' ') {
                all_space = false;
                break;
            }
        }
    } while (all_space);
    all_space = true;
    *left = -1;
    do {
        (*left)++;
        for (int i = 0; i < BUFFER_HEIGHT; i++) {
            if (buffer[i][*left] != ' ') {
                all_space = false;
                break;
            }
        }
    } while (all_space);
    all_space = true;
    *right = BUFFER_WIDTH;
    do {
        (*right)--;
        for (int i = 0; i < BUFFER_HEIGHT; i++) {
            if (buffer[i][*right] != ' ') {
                all_space = false;
                break;
            }
        }
    } while (all_space);
}

void write_to_file(char *path, char **card) {
    FILE *file = fopen(path, "w");
    char *buffer = (char *)malloc(sizeof(char) * (CARD_WIDTH + 3));
    buffer[0] = '|';
    buffer[CARD_WIDTH + 1] = '|';
    buffer[CARD_WIDTH + 2] = 0;
    memset(&buffer[1], '-', sizeof(char) * CARD_WIDTH);
    for (int i = 1; i < CARD_WIDTH + 1; i++) {
        buffer[i] = '-';
    }
    fprintf(file, "%s\n", buffer);
    for (int i = 0; i < CARD_HEIGHT; i++) {
        strncpy(&buffer[1], card[i], CARD_WIDTH);
        fprintf(file, "%s\n", buffer);
    }
    memset(&buffer[1], '-', sizeof(char) * CARD_WIDTH);
    fprintf(file, "%s\n", buffer);
    fclose(file);
}

bool check_extension(char *path) {
    int extension_len = strlen(CARD_EXTENSION);
    int path_len = strlen(path);
    if (path_len < extension_len || strcmp(&path[path_len - extension_len], CARD_EXTENSION)) {
        return false;
    }
    return true;
}

// draw card to file
void draw(char *path) {
    if (!check_extension(path)) {
        fprintf(stderr, "warning: only file whose name ends with \"%s\" will be recognized\n", CARD_EXTENSION);
    }
    if (exist_file(path)) {
        // file exists, check whether to overwrite
        printf("file exists, do you want to overwrite it? [y/n]\n");
        char reply[8];
        fgets(reply, 8, stdin);
        if (strcmp(reply, "y\n")) {
            exit(0);
        }
    }
    printf("please type in card content and press ^D when finished, redundant space will be removed\n");
    printf("");
    // allocate
    char **card = (char **)salloc(sizeof(char), (int[]){ CARD_HEIGHT, CARD_WIDTH }, 2);
    for (int i = 0; i < CARD_HEIGHT; i++) {
        memset(card[i], ' ', sizeof(char) * CARD_WIDTH);
    }
    char **buffer = (char **)salloc(sizeof(char), (int[]){ BUFFER_HEIGHT, BUFFER_WIDTH + 2 }, 2);
    for (int i = 0; i < BUFFER_HEIGHT; i++) {
        memset(buffer[i], ' ', sizeof(char) * (BUFFER_WIDTH + 2));
    }
    read_content(card, buffer);
    int content_top;
    int content_bottom;
    int content_left;
    int content_right;
    locate(buffer, &content_top, &content_bottom, &content_left, &content_right);
    // check whether content smaller or equal than card
    int content_height = content_bottom - content_top + 1;
    int content_width = content_right - content_left + 1;
    if (content_height > CARD_HEIGHT || content_width > CARD_WIDTH) {
        fprintf(stderr, "error: content larger than card\n");
        exit(1);
    }
    // copy content to card
    int card_top = (CARD_HEIGHT - content_height) / 2;
    int card_left = (CARD_WIDTH - content_width) / 2;
    for (int i = 0; i < content_height; i++) {
        strncpy(&card[card_top + i][card_left], &buffer[content_top + i][content_left], content_width);
    }
    write_to_file(path, card);
    free(card);
    free(buffer);
}

int main(int argc, char **argv) {
    arg_valid(argc, argv);
    char *path = argv[1];
    draw(path);
    return 0;
}
