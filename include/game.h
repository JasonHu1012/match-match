#ifndef GAME_H
#define GAME_H

#define PLAYER_NUM 2
#define CARD_NUM 5
#define SAME_NUM 2
#define CORRECT_SCORE 10
#define WRONG_SCORE 1
#define ICON_PATH "resource/icon.txt"
#define CARD_DIR "resource/card"
#define CARD_EXTENSION ".card"
#define CARD_PER_ROW 10

void load_resource();
void show_icon(int player_fd);
void write_waiting_message(int player_fd, int count);
void run_game(int *player_fd);

#endif
