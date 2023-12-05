#include <stdio.h>
#include <string.h>
#include <softTone.h>
#include <wiringPi.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include "mcp3008/mcp3008.h"


/* 방향키, 회전키 설정*/
#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define UP 3
#define CLICK 6

#define KEYPAD_PB1 16   // back , 5
#define KEYPAD_PB2 17  // click , 6

#define MAX_KEY_BT_NUM 2 // KEYPAD 버튼 개수 정의

#define BUZZER_PIN 26


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

void Change_FREQ(unsigned int freq) // 주파수를 변경하는 함수
{
    softToneWrite(BUZZER_PIN, freq); // Buzzer 핀에 주어진 주파수만큼 할당
}

void STOP_FREQ(void) // 주파수 발생 정지 함수
{
    softToneWrite(BUZZER_PIN, 0); // Buzzer 핀에 주파수 0 Hz 할당
}

void BUZZER_Init(void) // Buzzer 초기 설정
{
    softToneCreate(BUZZER_PIN); // Buzzer 핀에 주파수 신호 출력 설정
    STOP_FREQ();                // 주파수 발생 정지
}

void printCentered(const char *str, int width)
{
    int len = strlen(str);
    int padding = (width - len) / 2;
    for (int i = 0; i < padding; i++)
    {
        printf(" ");
    }
    printf("%s\n", str);
}

int readKey(){
    key keyinput = mcp3008();
	
	if(keyinput.x>900) return LEFT;
	else if(keyinput.x<10) return RIGHT;
	if(keyinput.y<10) return DOWN;
	else if(keyinput.y>900) return UP;

    return -1;
}

/*  Carve the maze starting at x, y. */
void CarveMaze(char *maze, int width, int height, int x, int y) {

   int x1, y1;
   int x2, y2;
   int dx, dy;
   int dir, count;

   dir = rand() % 4;
   count = 0;
   while(count < 4) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      x1 = x + dx;
      y1 = y + dy;
      x2 = x1 + dx;
      y2 = y1 + dy;
      if(   x2 > 0 && x2 < width && y2 > 0 && y2 < height
         && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
         maze[y1 * width + x1] = 0;
         maze[y2 * width + x2] = 0;
         x = x2; y = y2;
         dir = rand() % 4;
         count = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
      }
   }

}

/* Generate maze in matrix maze with size width, height. */
void GenerateMaze(char *maze, int width, int height) {

   int x, y;

   /* Initialize the maze. */
   for(x = 0; x < width * height; x++) {
      maze[x] = 1;
   }
   maze[1 * width + 1] = 0;

   /* Seed the random number generator. */
   srand(time(0));

   /* Carve the maze. */
   for(y = 1; y < height; y += 2) {
      for(x = 1; x < width; x += 2) {
         CarveMaze(maze, width, height, x, y);
      }
   }

   /* Set up the entry and exit. */
   maze[0 * width + 1] = 0;
   maze[(height - 1) * width + (width - 2)] = 0;

}

/* Solve the maze. */
void SolveMaze(char *maze, int width, int height) {

   int dir, count;
   int x, y;
   int dx, dy;
   int forward;

   /* Remove the entry and exit. */
   maze[0 * width + 1] = 1;
   maze[(height - 1) * width + (width - 2)] = 1;

   forward = 1;
   dir = 0;
   count = 0;
   x = 1;
   y = 1;
   while(x != width - 2 || y != height - 2) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      if(   (forward  && maze[(y + dy) * width + (x + dx)] == 0)
         || (!forward && maze[(y + dy) * width + (x + dx)] == 2)) {
         maze[y * width + x] = forward ? 2 : 3;
         x += dx;
         y += dy;
         forward = 1;
         count = 0;
         dir = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
         if(count > 3) {
            forward = 0;
            count = 0;
         }
      }
   }

   /* Replace the entry and exit. */
   maze[(height - 2) * width + (width - 2)] = 2;
   maze[(height - 1) * width + (width - 2)] = 2;

}
// 사용자의 현재 위치를 표시하는 함수
void ShowMaze(const char *maze, int width, int height, int player_x, int player_y) {
    for(int y = 0; y <= height; y++) {
        for(int x = 0; x <= width; x++) {
            if(x == player_x && y == player_y)
                printf("P ");
            else if(maze[y * width + x] == 1)
                printf("[]");
            else if(y==height)
                  printf("[]");
            else
                printf("  ");
        }
        printf("\n");
    }
}


// 사용자의 이동을 처리하는 함수
int MovePlayer(char *maze, int width, int height, int *player_x, int *player_y, char direction) {
    int dx = 0, dy = 0;
    switch(direction) {
        case UP: dy = -1; break; // 위로 이동
        case DOWN: dy = 1;  break; // 아래로 이동
        case LEFT: dx = -1; break; // 왼쪽으로 이동
        case RIGHT: dx = 1;  break; // 오른쪽으로 이동
    }
    int new_x = *player_x + dx;
    int new_y = *player_y + dy;
    if(new_x >= 0 && new_x < width && new_y >= 0 && new_y < height && maze[new_y * width + new_x] == 0) {
        *player_x = new_x;
        *player_y = new_y;
        if(*player_x == width - 1 && *player_y == height - 1){
            return 2; // 미로 탈출 성공
        }

        return 1; // 이동 성공
    }
    return 0; // 이동 실패
}

int Maze_Start(int music) {
    int i;
    int width = 10; // 너비
    int height = 10; // 높이
    int level = 1; // 레벨
    int score = 0; // 점수
    int state;
    int inkey;
    int nKeypadstate;
    char *maze = malloc(width * height);
    if(maze == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    GenerateMaze(maze, width, height);
    int player_x = 1, player_y = 1; // 플레이어 시작 위치

    char input;
    do {
        system("clear");
        printf("Level: %d  score: %d\n", level,score);
        ShowMaze(maze, width, height, player_x, player_y);
        nKeypadstate = KeypadRead();
        inkey = -1;                  // 입력키에 -1 저장
        inkey = readKey();

        if(inkey == -1) {
            for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
            {
                if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                {
                    inkey = i+5;
                }else{
                    inkey = -1;
                }
            }
            
        }

        state = MovePlayer(maze, width, height, &player_x, &player_y, inkey);
        if(state == 2){
            printf("미로 탈출 성공!\n");
            score += 100;
            level++;
            width += 2;
            height += 2;
            GenerateMaze(maze, width, height);
            player_x = 1;
            player_y = 1;
        }
    } while(state == 3); // 'q'를 누를 때까지 반복

    free(maze);
    return 0;
}

int main()
{
    int i;
    int cursor = 1;
    int select = 0;
    int sound = 1;
    int music = 1;
    int inkey;
    int nKeypadstate;
    
    if (wiringPiSetupGpio() == -1) // Wiring Pi의 GPIO를 사용하기 위한 설정
    {
        return 1; // Wiring Pi의 GPIO가 설정되지 않았다면 종료
    }

    for (i = 0; i < MAX_KEY_BT_NUM; i++) // KEYPAD 핀 입력 설정
    {
        pinMode(KeypadTable[i], INPUT); // KEYPAD 핀 테이블의 i번째 위치에 입력 설정 할당
    }

    while (1)
    {
        // 터미널 크기를 얻습니다.
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int terminal_width = w.ws_col;


        nKeypadstate = KeypadRead();
        inkey = -1;                  // 입력키에 -1 저장
        inkey = readKey();

        if(inkey == -1) {
            for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
            {
                if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                {
                    inkey = i+5;
                }else{
                    inkey = -1;
                }
            }
            
        }
        
        if(inkey != -1 && sound == 1){
            Change_FREQ(440);  // 440Hz 주파수 소리 재생
            usleep(100000);   // 0.1초 대기 (100000 마이크로초)
            STOP_FREQ();      // 소리 정지
        }

        // 화면을 지우고
        system("clear");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");


        // 텍스트를 중앙에 출력합니다.
        printCentered("\t\t\t\t\t\t\t\t\t□□□■■□□□□□■□□□□■■□□□□■□□□□□□■□□□□□■□□□□□□□□□□□□□□■□□■□□□□□□□■□□□□□□□□□□□■□■□□■■■■■■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■■■■■■□□□■□□□□■■□□□□■□□□■■■■■■■□□■□□□□□■■■■■■□□□■□□■□□□□□□□■□□□□■■■■■□□■□■□□■■□□■■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■■□□■■□□□■□□□□■■□□□□■□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□□■□□■□■□□■□□□□■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■□□□□■□□□■□□□□■■■□□□■■■□□■■■■■□□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■□□■□■□□■□□□□■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■□□□□■□□□■□□□■■■■■□□■□□□■■□□□■■□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■□□■■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■□□□□■□□□■□■■■□□■■■□■□□□■■□□□■■□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■■■■■□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■■□□□■□□□■□■■□□□□□□□■□□□□■■■■■□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□■■□□□■□■□□□□□□□□□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□■■□□■■□□□■□□□□■■■■■■■□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□■■■□■□□□□□■■■□□□■□■□□□□■■■■■■■■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□□■■■■■□□□■□□□□■■□□□■■□□□□□■□□□□□□■□□□□□■■■■■■□□□■□□■■■■■■■□■□□□□■■□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□□□□□□□□□□■□□□□■□□□□□■■□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□■□□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□□□□□□□□□□■□□□□■■□□□■■□□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("\t\t\t\t\t\t\t\t\t□□□□□□□□□□■□□□□■■■■■■■□□□□□■■■■■■■■□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■■■■■■■■", terminal_width);

        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");
        printf("\n");


        if (select == 0)
        {
            if (cursor == 1)
            {
                printCentered("1. 게임 목록 O", terminal_width);
                printCentered("2. 게임 순위", terminal_width);
                printCentered("3. 소리 설정", terminal_width);
                printCentered("4. 게임 종료", terminal_width);
            }
            else if (cursor == 2)
            {
                printCentered("1. 게임 목록", terminal_width);
                printCentered("2. 게임 순위 O", terminal_width);
                printCentered("3. 소리 설정", terminal_width);
                printCentered("4. 게임 종료", terminal_width);
            }
            else if (cursor == 3)
            {
                printCentered("1. 게임 목록", terminal_width);
                printCentered("2. 게임 순위", terminal_width);
                printCentered("3. 소리 설정 O", terminal_width);
                printCentered("4. 게임 종료", terminal_width);
            }
            else if (cursor == 4)
            {
                printCentered("1. 게임 목록", terminal_width);
                printCentered("2. 게임 순위", terminal_width);
                printCentered("3. 소리 설정", terminal_width);
                printCentered("4. 게임 종료 O", terminal_width);
            }
            if (inkey != -1 && inkey == DOWN)
            {
                cursor++;
                if (cursor > 4)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == UP)
            {
                cursor--;
                if (cursor < 1)
                {
                    cursor = 4;
                }
            }

            if (inkey != -1 && inkey == CLICK)
            {
                select = cursor;
                cursor = 1;
            }
        }
        else if (select == 1)
        {
            if (cursor == 1)
            {
                printCentered("1. 테트리스 O", terminal_width);
                printCentered("2. 미로 찾기", terminal_width);
            }
            else if (cursor == 2)
            {
                printCentered("1. 테트리스", terminal_width);
                printCentered("2. 미로 찾기 O", terminal_width);
            }
            if (inkey != -1 && inkey == CLICK && cursor == 1)
            {
                // 테트리스 게임 실행

                select = 0;
                cursor = 1;
            }
            else if (inkey != -1 && inkey == CLICK && cursor == 2)
            {
                // 미로 찾기 게임 실행
                Maze_Start(music);

                select = 0;
                cursor = 1;
            }

            if (inkey != -1 && inkey == 5)
            {
                select = 0;
                cursor = 1;
            }
            if (inkey != -1 && inkey == DOWN)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == UP)
            {
                cursor--;
                if (cursor < 1)
                {
                    cursor = 2;
                }
            }
        }
        if (select == 2)
        {
            if (cursor == 1)
            {
                printCentered("1. 테트리스 순위 O", terminal_width);
                printCentered("2. 미로 찾기 순위", terminal_width);
            }
            else if (cursor == 2)
            {
                printCentered("1. 테트리스 순위", terminal_width);
                printCentered("2. 미로 찾기 순위 O", terminal_width);
            }

            if (inkey != -1 && inkey == CLICK && cursor == 1)
            {
                //테트리스 게임 순위 출력

                select = 0;
                cursor = 1;
            }
            else if (inkey != -1 && inkey == CLICK && cursor == 2)
            {
                // 미로 찾기 게임 순위 출력

                select = 0;
                cursor = 1;
            }

            if (inkey != -1 && inkey == 5)
            {
                select = 0;
                cursor = 1;
            }
            if (inkey != -1 && inkey == DOWN)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == UP)
            {
                cursor--;
                if (cursor < 1)
                {
                    cursor = 2;
                }
            }
        }
        else if (select == 3)
        {
            if (sound == 1 && music == 1 && cursor == 1)
            {
                printCentered("1. 소리 ON O", terminal_width);
                printCentered("2. 음악 ON ", terminal_width);
            }
            else if (sound == 1 && music == 0 && cursor == 1)
            {
                printCentered("1. 소리 ON O", terminal_width);
                printCentered("2. 음악 OFF ", terminal_width);
            }
            else if (sound == 0 && music == 1 && cursor == 1)
            {
                printCentered("1. 소리 OFF O", terminal_width);
                printCentered("2. 음악 ON ", terminal_width);
            }
            else if (sound == 0 && music == 0 && cursor == 1)
            {
                printCentered("1. 소리 OFF O", terminal_width);
                printCentered("2. 음악 OFF ", terminal_width);
            }
            else if (sound == 1 && music == 1 && cursor == 2)
            {
                printCentered("1. 소리 ON ", terminal_width);
                printCentered("2. 음악 ON O", terminal_width);
            }
            else if (sound == 1 && music == 0 && cursor == 2)
            {
                printCentered("1. 소리 ON ", terminal_width);
                printCentered("2. 음악 OFF O", terminal_width);
            }
            else if (sound == 0 && music == 1 && cursor == 2)
            {
                printCentered("1. 소리 OFF ", terminal_width);
                printCentered("2. 음악 ON O", terminal_width);
            }
            else if (sound == 0 && music == 0 && cursor == 2)
            {
                printCentered("1. 소리 OFF ", terminal_width);
                printCentered("2. 음악 OFF O", terminal_width);
            }

            if (inkey != -1 && inkey == CLICK && cursor == 1)
            {
                if (sound == 1)
                {
                    sound = 0;
                }
                else if (sound == 0)
                {
                    sound = 1;
                }
            }
            else if (inkey != -1 && inkey == CLICK && cursor == 2)
            {
                if (music == 1)
                {
                    music = 0;
                }
                else if (music == 0)
                {
                    music = 1;
                }
            }
            if (inkey != -1 && inkey == 5)
            {
                select = 0;
                cursor = 1;
            }
            if (inkey != -1 && inkey == DOWN)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == UP)
            {
                cursor--;
                if (cursor < 1)
                {
                    cursor = 2;
                }
            }
        }
        else if(select == 4)
        {
            break;
        }

        printf("%d \n", inkey);
        // 일정 시간 동안 대기합니다 (예: 0.25초).
        delay(250);
    }

    return 0;
}
