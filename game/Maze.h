#ifndef MAZE_H
#define MAZE_H

#define LENGTH 20
#define TRUE 1
#define FALSE 0

#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define UP 3
#define CLICK 4

#define KEYPAD_PB1 16   // back , 5
#define KEYPAD_PB2 17  // pause , 6

#define MAX_KEY_BT_NUM 2 // KEYPAD 버튼 개수 정의

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <softTone.h>
#include <wiringPi.h>
#include "../mcp3008/mcp3008.h"

typedef struct _Room
{
    short visit;
}Room;

typedef struct _Map
{
    short empty;
    short visit;
}Map;

typedef struct _Path
{
    int x;
    int y;
}Path;

extern Room room[LENGTH][LENGTH];
extern Map bgrd[2 * LENGTH + 1][2 * LENGTH + 1];
extern Path path[LENGTH * LENGTH];

void Make_Map();
void Visit_Room(int x, int y);
void Display(int player_x, int player_y, int goal_x, int goal_y);
void Player();
void Maze_Start(int music);

#endif //MAZE_H
