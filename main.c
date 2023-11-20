#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <softTone.h>
#include <wiringPi.h>
#include "game/Tetirs.h"

#define KEYPAD_PB1 3  // KEYPAD 포트 BT1 핀 정의 , up
#define KEYPAD_PB2 17 // KEYPAD 포트 BT2 핀 정의 , down
#define KEYPAD_PB3 27 // KEYPAD 포트 BT3 핀 정의 , left
#define KEYPAD_PB4 18 // KEYPAD 포트 BT4 핀 정의 , right
#define KEYPAD_PB5 27 // KEYPAD 포트 BT5 핀 정의 , ok
#define KEYPAD_PB6 22 // KEYPAD 포트 BT6 핀 정의 , back

#define MAX_KEY_BT_NUM 6 // KEYPAD 버튼 개수 정의

const int KeypadTable[6] = {KEYPAD_PB1, KEYPAD_PB2, KEYPAD_PB3, KEYPAD_PB4, KEYPAD_PB5, KEYPAD_PB6}; // KEYPAD 핀 테이블 선언

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

int main()
{
    int i;
    int cursor = 1;
    int select = 0;
    int sound = 1;
    int music = 1;
    char inkey;
    int nKeypadstate;

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

        nKeypadstate = KeypadRead(); // KEYPAD로부터 버튼 입력을 읽어 상태를 변수에 저장
        inkey = -1;                  // 입력키에 -1 저장

        for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
        {
            if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
            {
                inkey = i;            // 입력키에 i 저장
                if (sound == 1)       // 입력키에 i 저장
                    Change_FREQ(500); // 버튼 누를때마다 '낮은 도' 출력
                delay(100);           // 10ms 동안 지속
                STOP_FREQ();          // 소리 출력 정지
            }
        }

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
            if (inkey != -1 && inkey == 0)
            {
                cursor++;
                if (cursor > 4)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == 1)
            {
                cursor--;
                if (cursor < 1)
                {
                    cursor = 4;
                }
            }

            if (inkey != -1 && inkey == 4)
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
            if (inkey != -1 && inkey == 4 && cursor == 1)
            {
                game_start(music);

                select = 0;
                cursor = 1;
            }
            else if (inkey != -1 && inkey == 4 && cursor == 2)
            {
                // 미로 찾기 게임실행

                select = 0;
                cursor = 1;
            }

            if (inkey != -1 && inkey == 5)
            {
                select = 0;
                cursor = 1;
            }
            if (inkey != -1 && inkey == 0)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == 1)
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

            if (inkey != -1 && inkey == 4 && cursor == 1)
            {
                // 테트리스 게임 순위 출력

                select = 0;
                cursor = 1;
            }
            else if (inkey != -1 && inkey == 4 && cursor == 2)
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
            if (inkey != -1 && inkey == 0)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == 1)
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

            if (inkey != -1 && inkey == 4 && cursor == 1)
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
            else if (inkey != -1 && inkey == 4 && cursor == 2)
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
            if (inkey != -1 && inkey == 0)
            {
                cursor++;
                if (cursor > 2)
                {
                    cursor = 1;
                }
            }
            else if (inkey != -1 && inkey == 1)
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
