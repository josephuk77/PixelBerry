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

#include "Maze.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <softTone.h>
#include <wiringPi.h>
#include "../mcp3008/mcp3008.h"

const int KeypadTable[12] = {KEYPAD_PB1, KEYPAD_PB2};                               // KEYPAD 핀 테이블 선언

int KeypadRead(void) // KEYPAD 버튼 눌림을 확인하여 nKeypadstate 상태 변수에 값을 저장
{
    int i = 0;            // 반복문 사용을 위한 정수형 변수 i, 0으로 초기화
    int nKeypadstate = 0; // KEYPAD 상태를 위한 정수형 변수 nKeypadstate, 0으로 초기화

    for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 5 미만까지 +1, 아래 코드 반복
    {
        if (!digitalRead(KeypadTable[i])) // 조건문 if, KEYPAD 핀 테이블의 i번째 위치 신호를 읽어서 반전했을때(버튼의 동작 범위 지정)
        {
            nKeypadstate |= (1 << i); // 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 OR연산 후 그 결과값을 저장
        }
    }

    return nKeypadstate; // KEYPAD 상태 값 반환
}

int readKey(){
    key keyinput = mcp3008();
	
	if(keyinput.clk<10) return CLICK;
	if(keyinput.x>900) return LEFT;
	else if(keyinput.x<10) return RIGHT;
	if(keyinput.y<10) return DOWN;
	else if(keyinput.y>900) return UP;

    return -1;
}

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

Room room[LENGTH][LENGTH] = { {FALSE}, };
Map bgrd[2 * LENGTH + 1][2 * LENGTH + 1] = { {FALSE, FALSE}, };
Path path[LENGTH * LENGTH] = { {0, 0}, };

void Make_Map();
void Visit_Room(int x, int y);
void Display(int player_x, int player_y, int goal_x, int goal_y);
void Player();

void Visit_Room(int x, int y)
{
    room[y][x].visit = TRUE;
}

void Make_Map()
{
    for (int i = 0; i < LENGTH; i++)
    {
        for (int j = 0; j < LENGTH; j++)
        {
            bgrd[2 * i + 1][2 * j + 1].empty = TRUE;
        }
    }

    int path_count = 0;

    srand((long)time(NULL));

    int current_x = rand() % LENGTH;
    int current_y = rand() % LENGTH;
    int previous_x;
    int previous_y;
    
    Visit_Room(current_x, current_y);
    path[path_count].x = current_x;
    path[path_count].y = current_y;
    path_count++;

    // 반복
    // 방 랜덤 선택

    while(path_count != 0)
    {   
        short available_room = 0;
        int i = 0;
        Path rand_room_list[4] = { {0,0}, };
        previous_x = current_x, previous_y = current_y;

        if (current_x > 0 && room[current_y][current_x - 1].visit == FALSE) //왼쪽 검사
        {
            rand_room_list[i].x = current_x - 1, rand_room_list[i].y = current_y;
            available_room++;
            i++;
        }
        if (current_x < LENGTH - 1 && room[current_y][current_x + 1].visit == FALSE) //오른쪽 검사
        {
            rand_room_list[i].x = current_x + 1, rand_room_list[i].y = current_y;
            available_room++;
            i++;
        }
        if (current_y > 0 && room[current_y - 1][current_x].visit == FALSE) //위쪽 검사
        {
            rand_room_list[i].x = current_x, rand_room_list[i].y = current_y - 1;
            available_room++;
            i++;
        }
        if (current_y < LENGTH - 1 && room[current_y + 1][current_x].visit == FALSE) //아래쪽 검사
        {
            rand_room_list[i].x = current_x, rand_room_list[i].y = current_y + 1;
            available_room++;
            i++;
        }

        if (available_room != 0)
        {
            short rand_num = rand() % available_room;
            current_x = rand_room_list[rand_num].x, current_y = rand_room_list[rand_num].y;

            Visit_Room(current_x, current_y);
            path[path_count].x = current_x;
            path[path_count].y = current_y;
            path_count++;

            int move_x = current_x - previous_x, move_y = current_y - previous_y;

            if (move_x == 0)
            {
                bgrd[2 * previous_y + 1 + move_y][2 * previous_x + 1].empty = TRUE;
            }
            else
            {
                bgrd[2 * previous_y + 1][2 * previous_x + 1 + move_x].empty = TRUE;
            }
        }
        else 
        {
            path[path_count].x = 0;
            path[path_count].y = 0;
            path_count--;
            current_x = path[path_count].x, current_y = path[path_count].y;
        }
    }
}

void Display(int player_x, int player_y, int goal_x, int goal_y)
{
    clear();
    for (int m = 0; m < 2 * LENGTH + 1; m++)
    {
        for (int n = 0; n < 2 * LENGTH + 1; n++)
        {
            if (bgrd[m][n].empty == TRUE)
            {
                if (m == player_y && n == player_x)
                {
                    attron(COLOR_PAIR(1)); // 초록색 쌍 활성화
                    printw("@");
                    attroff(COLOR_PAIR(1)); // 초록색 쌍 비활성화
                }
                else if (m == goal_y && n == goal_x)
                {
                    attron(COLOR_PAIR(2)); // 노란색 쌍 활성화
                    printw("*");
                    attroff(COLOR_PAIR(2)); // 노란색 쌍 비활성화
                }
                else if (bgrd[m][n].visit == TRUE)
                {
                    printw(".");
                }
                else
                {
                    printw("  ");
                }
            }
            else
            {
                printw("#");
            }
        }
        printw("
");
    }
}


void Player()
{
    int inkey;
    int nKeypadstate;
    int player_x = 1, player_y = 1;
    int goal_x = 2 * LENGTH - 1, goal_y = 2 * LENGTH - 1;
    bgrd[player_y][player_x].visit = TRUE;
    Display(player_x, player_y, goal_x, goal_y);

    while (player_x != goal_x || player_y != goal_y)
    {
        nKeypadstate = KeypadRead();
        inkey = -1;                  // 입력키에 -1 저장
        inkey = readKey();

        for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
        {
            if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
            {
                inkey = i+5;
            }
        }

        if (inkey == UP)
        {
            if (bgrd[player_y - 1][player_x].empty == TRUE)
            {
                player_y--;
                bgrd[player_y][player_x].visit = TRUE;
                Display(player_x, player_y, goal_x, goal_y);
            }
        }
        else if (inkey == DOWN)
        {
            if (bgrd[player_y + 1][player_x].empty == TRUE)
            {
                player_y++;
                bgrd[player_y][player_x].visit = TRUE;
                Display(player_x, player_y, goal_x, goal_y);
            }
        }
        else if (inkey == LEFT)
        {
            if (bgrd[player_y][player_x - 1].empty == TRUE)
            {
                player_x--;
                bgrd[player_y][player_x].visit = TRUE;
                Display(player_x, player_y, goal_x, goal_y);
            }
        }
        else if (inkey == RIGHT)
        {
            if (bgrd[player_y][player_x + 1].empty == TRUE)
            {
                player_x++;
                bgrd[player_y][player_x].visit = TRUE;
                Display(player_x, player_y, goal_x, goal_y);
            }
        }

        delay(200);
    }
    printw("도착!\n");
}

int Maze_Start(int music)
{
    if (wiringPiSetupGpio() == -1) // Wiring Pi의 GPIO를 사용하기 위한 설정
    {
        return 1; // Wiring Pi의 GPIO가 설정되지 않았다면 종료
    }

    for (i = 0; i < MAX_KEY_BT_NUM; i++) // KEYPAD 핀 입력 설정
    {
        pinMode(KeypadTable[i], INPUT); // KEYPAD 핀 테이블의 i번째 위치에 입력 설정 할당
    }

    initscr();
    start_color(); // 색상 기능 활성화
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // 1번 색상 쌍을 초록색/검정색으로 설정
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // 2번 색상 쌍을 노란색/검정색으로 설정

    noecho();
    cbreak();

    Make_Map();
    Player();

    endwin();
    return 0;
}
