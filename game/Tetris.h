#ifndef TETRIS_H
#define TETRIS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <softTone.h>
#include <wiringPi.h>

#define BUZZER_PIN 26

#define KEYPAD_PB1 3  // KEYPAD 포트 BT1 핀 정의 , up
#define KEYPAD_PB2 17 // KEYPAD 포트 BT2 핀 정의 , down
#define KEYPAD_PB3 27 // KEYPAD 포트 BT3 핀 정의 , left
#define KEYPAD_PB4 18 // KEYPAD 포트 BT4 핀 정의 , right
#define KEYPAD_PB5 27 // KEYPAD 포트 BT5 핀 정의 , ok
#define KEYPAD_PB6 22 // KEYPAD 포트 BT6 핀 정의 , back

#define MAX_KEY_BT_NUM 6 // KEYPAD 버튼 개수 정의

// 방향키, 회전키 설정
#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define ROTATE 3

// 테트로미노 블록넘버 설정
#define I_BLOCK 0
#define T_BLOCK 1
#define S_BLOCK 2
#define Z_BLOCK 3
#define L_BLOCK 4
#define J_BLOCK 5
#define O_BLOCK 6

// 게임 시작, 종료 설정
#define GAME_START 0
#define GAME_END 1

// 테트리스 판 크기 설정
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 21

// 테트리스 게임 상태 변수
extern int block_number;
extern int next_block_number;
extern int block_state;
extern int x, y; // 블록의 현재 위치
extern int game; // 게임 상태
extern long point; // 점수

// 테트리스 판 배열
extern char tetris_table[BOARD_HEIGHT][BOARD_WIDTH];

// 테트리스 블록 구조체
typedef struct {
    char shape[4][4][4];
} Tetromino;

// 결과 저장 구조체
typedef struct result {
    char name[30];
    long point;
    int year, month, day, hour, min;
} Result;

// 함수 선언
int game_start(int music);
void playNote(int note, int duration);
void* playMusic(void* arg);
int refresh(int signum);
int move_block(int command);
int collision_test(int command);
int drop(void);
int check_one_line(void);
int KeypadRead(void);
int display_tetris_table(void);
int init_tetris_table(void);
int print_result_tetris(void);

#endif /* TETRIS_H */
