#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include "game.h"
#include "func.h"

#define BUFFER_LEN 128
#define ROW 64
#define ICON_LEN 2048
#define CARD_LEN 2048
#define MINI_CARD_HEIGHT 6
#define MINI_CARD_WIDTH 7

#define ERR_EXIT(s) perror(s); exit(1);

typedef struct {
    int fd;
    int score;
    char *name;
} player_t;

typedef struct {
    int id;
    bool flip;
    char *content;
} card_t;

char icon_string[ICON_LEN];
char **all_card_string;
char buffer[BUFFER_LEN];
char table[ROW][BUFFER_LEN];
player_t player[PLAYER_NUM];
card_t card[CARD_NUM * SAME_NUM];
int total_card;

static int count_card(char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        ERR_EXIT("open card directory");
    }
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir))) {
        // skip "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        // recursively count cards in directory
        if (entry->d_type == DT_DIR) {
            sprintf(buffer, "%s/%s", dir_path, entry->d_name);
            count += count_card(buffer);
            continue;
        }
        if (entry->d_type == DT_REG) {
            // check file name extension
            int extension_len = strlen(CARD_EXTENSION);
            int name_len = strlen(entry->d_name);
            if (name_len < extension_len) {
                continue;
            }
            if (strcmp(&entry->d_name[name_len - extension_len], CARD_EXTENSION)) {
                continue;
            }
            count++;
        }
    }
    return count;
}

static void read_card(char *dir_path, int *index) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        ERR_EXIT("open card directory");
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        // skip "." and ".."
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
            continue;
        }
        // recursively read cards in directory
        if (entry->d_type == DT_DIR) {
            sprintf(buffer, "%s/%s", dir_path, entry->d_name);
            read_card(buffer, index);
            continue;
        }
        if (entry->d_type == DT_REG) {
            // check file name extension
            int extension_len = strlen(CARD_EXTENSION);
            int name_len = strlen(entry->d_name);
            if (name_len < extension_len) {
                continue;
            }
            if (strcmp(&entry->d_name[name_len - extension_len], CARD_EXTENSION)) {
                continue;
            }
            sprintf(buffer, "%s/%s", dir_path, entry->d_name);
            int card_fd = open(buffer, O_RDONLY);
            int len = read(card_fd, all_card_string[(*index)++], CARD_LEN);
            if (len == -1) {
                ERR_EXIT("read card");
            }
        }
    }
}

// top left row index
static int get_row_index(int card_index) {
    return (card_index / CARD_PER_ROW) * MINI_CARD_HEIGHT;
}

// top left column index
static int get_col_index(int card_index) {
    return (card_index % CARD_PER_ROW) * MINI_CARD_WIDTH;
}

void load_resource() {
    // icon
    int icon_fd = open(ICON_PATH, O_RDONLY);
    if (icon_fd == -1) {
        ERR_EXIT("open icon");
    }
    int len = read(icon_fd, icon_string, ICON_LEN);
    if (len == -1) {
        ERR_EXIT("read icon");
    }
    icon_string[len] = 0;
    close(icon_fd);
    // card
    total_card = count_card(CARD_DIR);
    if (total_card < CARD_NUM) {
        sprintf(buffer, "number of cards in %s is less than %d", CARD_DIR, CARD_NUM);
        ERR_EXIT(buffer);
    }
    all_card_string = (char **)malloc(sizeof(char *) * total_card);
    for (int i = 0; i < total_card; i++) {
        all_card_string[i] = (char *)malloc(sizeof(char) * CARD_LEN);
    }
    int index = 0;
    read_card(CARD_DIR, &index);
    // table
    for (int i = 0; i < ROW; i++) {
        memset(table[i], 0, BUFFER_LEN);
    }
    for (int i = 0; i < CARD_NUM * SAME_NUM; i++) {
        int row = get_row_index(i);
        int col = get_col_index(i);
        strncpy(&table[row][col], "       ", MINI_CARD_WIDTH);
        strncpy(&table[row + 1][col], "       ", MINI_CARD_WIDTH);
        int num_len = sprintf(buffer, "%d", i + 1);
        strncpy(&table[row + 1][col + 3], buffer, strlen(buffer));
        strncpy(&table[row + 2][col], " |----|", MINI_CARD_WIDTH);
        strncpy(&table[row + 3][col], " |    |", MINI_CARD_WIDTH);
        strncpy(&table[row + 4][col], " |    |", MINI_CARD_WIDTH);
        strncpy(&table[row + 5][col], " |----|", MINI_CARD_WIDTH);
    }
    // append '\n' to each row in table
    for (int i = 0; i < ROW; i++) {
        int len = strlen(table[i]);
        if (!len) {
            break;
        }
        table[i][len] = '\n';
    }
}

static void clear_screen(int player_fd) {
    sprintf(buffer, "\033[H\033[J");
    int ret = write(player_fd, buffer, strlen(buffer));
    if (ret == -1) {
        ERR_EXIT("clear screen");
    }
}

void show_icon(int player_fd) {
    clear_screen(player_fd);
    int ret = write(player_fd, icon_string, strlen(icon_string));
    if (ret == -1) {
        ERR_EXIT("write icon");
    }
}

static void clear_line(int player_fd) {
    for (int i = 0; i < BUFFER_LEN; i++) {
        int ret = write(player_fd, "\b", 1);
        if (ret == -1) {
            ERR_EXIT("clear line");
        }
    }
}

void write_waiting_message(int player_fd, int count) {
    clear_line(player_fd);
    sprintf(buffer, "player number: %d/%d", count, PLAYER_NUM);
    int ret = write(player_fd, buffer, strlen(buffer));
    if (ret == -1) {
        ERR_EXIT("message (wait for other player to join)");
    }
}

static void prepare_game(int *player_fd) {
    srand(time(NULL));
    // card
    int *desired = distinct_rand(0, total_card - 1, CARD_NUM);
    int *shuffle = distinct_rand(0, CARD_NUM * SAME_NUM - 1, CARD_NUM * SAME_NUM);
    int index = 0;
    for (int i = 0; i < CARD_NUM; i++) {
        for (int j = 0; j < SAME_NUM; j++) {
            // initialize card
            card[shuffle[index]].id = i;
            card[shuffle[index]].flip = false;
            card[shuffle[index++]].content = all_card_string[desired[i]];
        }
    }
    free(desired);
    free(shuffle);
    // nickname
    struct pollfd read_poll[PLAYER_NUM];
    for (int i = 0; i < PLAYER_NUM; i++) {
        sprintf(buffer, "\nplease type in your nickname: ");
        int ret = write(player_fd[i], buffer, strlen(buffer));
        if (ret == -1) {
            ERR_EXIT("name message");
        }
        // initalize read_poll
        read_poll[i].fd = player_fd[i];
        read_poll[i].events = 0;
        read_poll[i].events |= POLLIN;
    }
    int ready_player = 0;
    int *order = distinct_rand(0, PLAYER_NUM - 1, PLAYER_NUM);
    while (ready_player < PLAYER_NUM) {
        int ret = poll(read_poll, PLAYER_NUM, -1);
        if (ret == -1) {
            ERR_EXIT("wait for name");
        }
        for (int i = 0; i < PLAYER_NUM; i++) {
            if (read_poll[i].revents & POLLIN) {
                read_poll[i].events &= ~POLLIN;
                player[order[i]].fd = player_fd[i];
                int len = read(player_fd[i], buffer, BUFFER_LEN);
                if (len == -1) {
                    ERR_EXIT("read name");
                }
                // change buffer[len - 1] from '\n' to '\0'
                buffer[len - 1] = '\0';
                player[order[i]].name = (char *)malloc(sizeof(char) * len);
                strncpy(player[order[i]].name, buffer, len);
                player[order[i]].score = 0;
                sprintf(buffer, "waiting for other players...");
                ret = write(player_fd[i], buffer, strlen(buffer));
                if (ret == -1) {
                    ERR_EXIT("message (wait for other player to type in nickname)");
                }
                ready_player++;
            }
            else if (read_poll[i].revents != 0) {
                ERR_EXIT("poll revent");
            }
        }
    }
    free(order);
}

static void show_table(int player_fd) {
    for (int i = 0; i < ROW; i++) {
        int ret = write(player_fd, table[i], strlen(table[i]));
        if (ret == -1) {
            ERR_EXIT("show table");
        }
    }
}

static void show_card(int player_fd, int card_index) {
    int ret = write(player_fd, card[card_index].content, strlen(card[card_index].content));
    if (ret == -1) {
        ERR_EXIT("show card");
    }
}

static void show_information(int player_index) {
    sprintf(buffer, "name: %s          score: %d\n", player[player_index].name, player[player_index].score);
    int ret = write(player[player_index].fd, buffer, strlen(buffer));
    if (ret == -1) {
        ERR_EXIT("show information");
    }
}

static int choose_card(int player_fd) {
    int choose;
    while (true) {
        sprintf(buffer, "please choose a card: ");
        int ret = write(player_fd, buffer, strlen(buffer));
        if (ret == -1) {
            ERR_EXIT("message (choose card)");
        }
        int len = read(player_fd, buffer, BUFFER_LEN);
        if (len == -1) {
            ERR_EXIT("read chosen card");
        }
        buffer[len - 1] = 0;
        if (!str_to_num(buffer, &choose)) {
            sprintf(buffer, "please type in an integer\n");
            ret = write(player_fd, buffer, strlen(buffer));
            if (ret == -1) {
                ERR_EXIT("message (type in integer)");
            }
            continue;
        }
        if (choose < 1 || choose > CARD_NUM * SAME_NUM) {
            sprintf(buffer, "exceed range (1 ~ %d)\n", CARD_NUM * SAME_NUM);
            ret = write(player_fd, buffer, strlen(buffer));
            if (ret == -1) {
                ERR_EXIT("message (exceed range)");
            }
            continue;
        }
        if (card[choose - 1].flip) {
            sprintf(buffer, "already flipped\n");
            ret = write(player_fd, buffer, strlen(buffer));
            if (ret == -1) {
                ERR_EXIT("message (already flipped)");
            }
            continue;
        }
        break;
    }
    return choose;
}

static void start_game() {
    int current_player = 0;
    int remaining = CARD_NUM;
    int pick[SAME_NUM];
    while (remaining) {
        for (int i = 0; i < SAME_NUM; i++) {
            for (int j = 0; j < PLAYER_NUM; j++) {
                clear_screen(player[j].fd);
                show_information(j);
                show_table(player[j].fd);
            }
            int choose = choose_card(player[current_player].fd) - 1;
            pick[i] = choose;
            card[choose].flip = true;
            int row = get_row_index(choose);
            int col = get_col_index(choose);
            strncpy(&table[row + 3][col + 2], "****", 4);
            strncpy(&table[row + 4][col + 2], "****", 4);
            for (int j = 0; j < PLAYER_NUM; j++) {
                clear_screen(player[j].fd);
                show_information(j);
                show_card(player[j].fd, choose);
            }
            sprintf(buffer, "press ENTER to continue\n");
            int ret = write(player[current_player].fd, buffer, strlen(buffer));
            if (ret == -1) {
                ERR_EXIT("message (press ENTER)");
            }
            ret = read(player[current_player].fd, buffer, BUFFER_LEN);
            if (ret == -1) {
                ERR_EXIT("press ENTER");
            }
        }
        // check whether the cards match
        bool match = true;
        for (int i = 1; i < SAME_NUM; i++) {
            if (card[pick[i - 1]].id != card[pick[i]].id) {
                match = false;
                break;
            }
        }
        if (match) {
            player[current_player].score += CORRECT_SCORE;
            for (int i = 0; i < SAME_NUM; i++) {
                int row = get_row_index(pick[i]);
                int col = get_col_index(pick[i]);
                strncpy(&table[row + 1][col], "       ", MINI_CARD_WIDTH);
                strncpy(&table[row + 2][col], "       ", MINI_CARD_WIDTH);
                strncpy(&table[row + 3][col], "       ", MINI_CARD_WIDTH);
                strncpy(&table[row + 4][col], "       ", MINI_CARD_WIDTH);
                strncpy(&table[row + 5][col], "       ", MINI_CARD_WIDTH);
            }
            remaining--;
        }
        else {
            player[current_player].score -= WRONG_SCORE;
            // let minimum score be zero
            if (player[current_player].score < 0) {
                player[current_player].score = 0;
            }
            for (int i = 0; i < SAME_NUM; i++) {
                card[pick[i]].flip = false;
                int row = get_row_index(pick[i]);
                int col = get_col_index(pick[i]);
                strncpy(&table[row + 3][col + 2], "    ", 4);
                strncpy(&table[row + 4][col + 2], "    ", 4);
            }
        }
        current_player = (current_player + 1) % PLAYER_NUM;
    }
}

static void show_score(int player_fd, player_t *rank) {
    clear_screen(player_fd);
    sprintf(buffer, "|-------------|\n| SCORE BOARD |\n|-------------|\n");
    int ret = write(player_fd, buffer, strlen(buffer));
    if (ret == -1) {
        ERR_EXIT("write score");
    }
    for (int i = 0; i < PLAYER_NUM; i++) {
        sprintf(buffer, "  %s: %d\n", rank[i].name, rank[i].score);
        ret = write(player_fd, buffer, strlen(buffer));
        if (ret == -1) {
            ERR_EXIT("write score");
        }
    }
}

int player_cmp(void const *a, void const *b) {
    return ((player_t *)b)->score - ((player_t *)a)->score;
}

static void release_memory() {
    for (int i = 0; i < PLAYER_NUM; i++) {
        free(player[i].name);
    }
    for (int i = 0; i < total_card; i++) {
        free(all_card_string[i]);
    }
    free(all_card_string);
}

static void end_game() {
    player_t rank[PLAYER_NUM];
    memcpy(rank, player, sizeof(player_t) * PLAYER_NUM);
    qsort(rank, PLAYER_NUM, sizeof(player_t), player_cmp);
    for (int i = 0; i < PLAYER_NUM; i++) {
        show_score(player[i].fd, rank);
        close(player[i].fd);
    }
    release_memory();
}

void run_game(int *player_fd) {
    prepare_game(player_fd);
    start_game();
    end_game();
}
