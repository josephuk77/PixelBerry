#include <stdio.h>
#include <string.h>
#include <softTone.h>
#include <wiringPi.h>
#include <pthread.h>
#include "mcp3008.h"


/* 방향키, 회전키 설정*/
#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define UP 3
#define CLICK 4

#define BUZZER_PIN 26

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
	
	if(keyinput.clk<10) return CLICK;
	if(keyinput.x>900) return LEFT;
	else if(keyinput.x<10) return RIGHT;
	if(keyinput.y<10) return DOWN;
	else if(keyinput.y>900) return UP;

    return -1;
}

int main()
{
    int i;
    int cursor = 1;
    int select = 0;
    int sound = 1;
    int music = 1;
    int inkey;
    
    if (wiringPiSetupGpio() == -1) // Wiring Pi의 GPIO를 사용하기 위한 설정
    {
        return 1; // Wiring Pi의 GPIO가 설정되지 않았다면 종료
    }


    while (1)
    {
        // 터미널 크기를 얻습니다.
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int terminal_width = w.ws_col;

        inkey = -1;                  // 입력키에 -1 저장
        inkey = readKey();

        // 화면을 지우고
        system("clear");
        printf("\n");
        printf("\n");

        // 텍스트를 중앙에 출력합니다.
        printCentered("□□□■■□□□□□■□□□□■■□□□□■□□□□□□■□□□□□■□□□□□□□□□□□□□□■□□■□□□□□□□■□□□□□□□□□□□■□■□□■■■■■■□□□■", terminal_width);
        printCentered("□■■■■■■□□□■□□□□■■□□□□■□□□■■■■■■■□□■□□□□□■■■■■■□□□■□□■□□□□□□□■□□□□■■■■■□□■□■□□■■□□■■□□□■", terminal_width);
        printCentered("□■■□□■■□□□■□□□□■■□□□□■□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□□■□□■□■□□■□□□□■□□□■", terminal_width);
        printCentered("□■□□□□■□□□■□□□□■■■□□□■■■□□■■■■■□□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■□□■□■□□■□□□□■□□□■", terminal_width);
        printCentered("□■□□□□■□□□■□□□■■■■■□□■□□□■■□□□■■□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■□□■■□□□■", terminal_width);
        printCentered("□■□□□□■□□□■□■■■□□■■■□■□□□■■□□□■■□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■■■■■□□□■", terminal_width);
        printCentered("□■■□□□■□□□■□■■□□□□□□□■□□□□■■■■■□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□■■□□□■□■□□□□□□□□□□□■", terminal_width);
        printCentered("□■■□□■■□□□■□□□□■■■■■■■□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□■■■□■□□□□□■■■□□□■□■□□□□■■■■■■■■", terminal_width);
        printCentered("□□■■■■■□□□■□□□□■■□□□■■□□□□□■□□□□□□■□□□□□■■■■■■□□□■□□■■■■■■■□■□□□□■■□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("□□□□□□□□□□■□□□□■□□□□□■■□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□■□□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("□□□□□□□□□□■□□□□■■□□□■■□□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■□□□□□□■", terminal_width);
        printCentered("□□□□□□□□□□■□□□□■■■■■■■□□□□□■■■■■■■■□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■■■■■■■■", terminal_width);

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
        if (select == 1)
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
        if (select == 3)
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
        if (select == 4)
        {
            break;
        }

        // 일정 시간 동안 대기합니다 (예: 0.25초).
        delay(250);
    }

    return 0;
}
