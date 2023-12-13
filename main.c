#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>
#include <softTone.h>
#include <wiringPi.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <ncurses.h>
#include "mcp3008/mcp3008.h"

#define TIMEOUT_DURATION 33
#define MESSAGE_DURATION 3
// Function to display the timeout message
void *displayTimeoutMessage(void *arg) {
    sleep(MESSAGE_DURATION);
    printf("\nTimeout! ");
    pthread_exit(NULL);
}

typedef struct {
    int playerId; // Unique player ID
    int level;
    int score;
} RankingEntry;


int numPlayers = 0;

/* 방향키, 회전키 설정*/
#define LEFT 0
#define RIGHT 1
#define DOWN 2
#define UP 3
#define CLICK 6

/* 타이머  */
#define CCHAR 0
#ifdef CTIME
#undef CTIME
#endif
#define CTIME 1

/* 테트리스 테트로 미노 블록넘버 설정*/
#define I_BLOCK 0
#define	T_BLOCK 1
#define S_BLOCK 2
#define Z_BLOCK 3
#define L_BLOCK 4
#define J_BLOCK 5
#define O_BLOCK 6

/* 테트리스 게임 시작, 종료 설정*/
#define GAME_START 0
#define GAME_END 1


#define KEYPAD_PB1 16 // back , 5
#define KEYPAD_PB2 17 // click , 6

#define MAX_KEY_BT_NUM 2 // KEYPAD 버튼 개수 정의

#define BUZZER_PIN 26
#define GAME_DURATION 40  // Set the game duration to 40 seconds
#define RANKING_FILE "ranking.txt"  // File to store player rankings

void UpdateRanking(const int playerId, int score, int level);
void displayMazeRankings();
void recordMazeRanking(const int playerId,int level, int score);

const int KeypadTable[2] = {KEYPAD_PB1, KEYPAD_PB2}; // KEYPAD 핀 테이블 선언

volatile int playMusicFlag = 1;
volatile int tetrisplayMusicFlag = 1;

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

void Buzzer_Init(void)
{
    softToneCreate(BUZZER_PIN);
    STOP_FREQ();
}

void playNote(int note, int duration)
{
    softToneWrite(BUZZER_PIN, note);
    delay(duration);
    softToneWrite(BUZZER_PIN, 0);
    delay(50); // 노트 사이의 짧은 휴식
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

void printCenteredMaze(const char* str, int terminal_width) {
    int len = strlen(str);
    int padding = (terminal_width - len) / 2;
    for (int i = 0; i < padding; i++) {
        printf(" ");
    }
    printf("%s\n", str);
}

void printFormattedCentered(int width, const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    printCentered(buffer, width);
}


int readKey()
{
    key keyinput = mcp3008();

    if (keyinput.x > 900)
        return LEFT;
    else if (keyinput.x < 10)
        return RIGHT;
    if (keyinput.y < 10)
        return DOWN;
    else if (keyinput.y > 900)
        return UP;

    return -1;
}

// Comparison function for qsort
int compareRankingEntries(const void *a, const void *b) {
    const RankingEntry *entryA = (const RankingEntry *)a;
    const RankingEntry *entryB = (const RankingEntry *)b;

    // Compare scores in descending order
    if (entryA->score > entryB->score) {
        return -1;
    } else if (entryA->score < entryB->score) {
        return 1;
    } else {
        // If scores are equal, compare levels in descending order
        if (entryA->level > entryB->level) {
            return -1;
        } else if (entryA->level < entryB->level) {
            return 1;
        } else {
            // If scores and levels are equal, compare player IDs in ascending order
            return entryA->playerId - entryB->playerId;
        }
    }
}

//**************************************************************
//*****************테트리스 게임***********************
//**************************************************************

int readKetetris()
{
    key keyinputtetris = mcp3008();

    if (keyinputtetris.x > 900) move_block(LEFT);
    else if (keyinputtetris.x < 10) move_block(RIGHT);
    if (keyinputtetris.y < 10) move_block(DOWN);
    else if (keyinputtetris.y > 900) move_block(UP);
}
char i_block[4][4][4] =
{
        1, 1, 1, 1,   0, 0, 0, 0,    0, 0, 0, 0,    0,0,0,0,
        0, 0, 0, 1,   0, 0, 0, 1,    0, 0, 0, 1,    0,0,0,1,
        0, 0, 0, 0,   0, 0, 0, 0,    0, 0, 0, 0,    1,1,1,1,
        1, 0, 0, 0,   1, 0, 0, 0,    1, 0, 0, 0,    1,0,0,0
};

char t_block[4][4][4] =
{
        1, 0, 0, 0,   1, 1, 0, 0,   1, 0, 0, 0,   0, 0, 0, 0,
        1, 1, 1, 0,   0, 1, 0, 0,   0, 0, 0, 0,   0, 0, 0, 0,
        0, 0, 1, 0,   0, 1, 1, 0,   0, 0, 1, 0,   0, 0, 0, 0,
        0, 0, 0, 0,   0, 1, 0, 0,   1, 1, 1, 0,   0, 0, 0, 0
};

char s_block[4][4][4] =
{
        1, 0, 0, 0,    1, 1, 0, 0,    0, 1, 0, 0,   0, 0, 0, 0,
        0, 1, 1, 0,    1, 1, 0, 0,    0, 0, 0, 0,   0, 0, 0, 0,
        0, 1, 0, 0,    0, 1, 1, 0,    0, 0, 1, 0,   0, 0, 0, 0,
        0, 0, 0, 0,    0, 1, 1, 0,    1, 1, 0, 0,   0, 0, 0, 0
};

char z_block[4][4][4] =
{
        0, 1, 0, 0,    1, 1, 0, 0,   1, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 0, 0,    0, 1, 1, 0,   0, 0, 0, 0,    0, 0, 0, 0,
        0, 0, 1, 0,    0, 1, 1, 0,   0, 1, 0, 0,    0, 0, 0, 0,
        0, 0, 0, 0,    1, 1, 0, 0,    0, 1, 1, 0,   0, 0, 0, 0
};

char l_block[4][4][4] =
{
        1, 0, 0, 0,    1, 0, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,
        1, 1, 1, 0,    1, 0, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
        0, 1, 1, 0,    0, 0, 1, 0,    0, 0, 1, 0,    0, 0, 0, 0,
        0, 0, 0, 0,    0, 0, 1, 0,    1, 1, 1, 0,    0, 0, 0, 0
};

char j_block[4][4][4] =
{
        0, 1, 0, 0,    0, 1, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,
        1, 0, 0, 0,    1, 1, 1, 0,    0, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 0, 0,    1, 0, 0, 0,    1, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 1, 0,    0, 0, 1, 0,    0, 0, 0, 0,    0, 0, 0, 0
};

char o_block[4][4][4] =
{
        1, 1, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
        1, 1, 0, 0,    1, 1, 0, 0,    0, 0, 0, 0,    0, 0, 0, 0,
};

char tetris_table[21][15];

static struct result
{
    char name[30];
    long point;
    int year;
    int month;
    int day;
    int hour;
    int min;
    int rank;
}temp_result;

int block_number = 0;  
int next_block_number = 0; 
int block_state = 0; 
int x = 3, y = 0; 
int game = GAME_END; 
int best_point = 0; 
long point = 0; 

int display_menu(void); 
int init_tetris_table(void); 
int display_tetris_table(void); 
int game_start(void); 
int refresh1(int);
int move_block(int);
int drop(void);
int collision_test(int); 
int check_one_line(void);
int print_result(void);
int search_result(void); 
int getch1(void);

int tetrisMelody[] = {
    659, 494, 523, 587, 523, 494, 440, 440, 523, 659, 587, 523, 494, 523, 587, 659, 523, 494, 440, 440, 523, 659, 587, 523, 494, 523, 587, 523, 494, 440, 494, 523, 587, 659, 784, 880, 784, 659, 587, 659, 523
};


int tetrisDurations[] = {
    300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300, 300
};

void *tetrisplayMusic(void *arg)
{
    int tetrismelodyLength = sizeof(tetrisMelody) / sizeof(int);
    while (tetrisplayMusicFlag)
    {
        for (int i = 0; i < tetrismelodyLength; i++)
        {
            if (!tetrisplayMusicFlag)
                break; // 노래 중지 확인
            int tetrisnote = tetrisMelody[i];
            int tetrisduration = tetrisnoteDurations[i];
            playNote(tetrisnote, tetrisduration);
        }
    }
    return NULL;
}

void* thread_function(void* arg) {
    while (game != GAME_END) {
        refresh1(1); // Replace with your function
        usleep(10000); // 10 milliseconds
    }
    return NULL;
}

int game_start(void) {
    pthread_t thread_id;
    time_t ptime;
    struct tm* t;
    FILE* fp = NULL;

    if (game == GAME_START) {
        pthread_t tetrismusicThread;
    
    if (maze == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }
    softToneCreate(BUZZER_PIN);

    if (music == 1)
    {
        pthread_create(&musicThread, NULL, playMusic, NULL);
    }
        init_tetris_table();
        // Create a thread to run the function periodically
        pthread_create(&thread_id, NULL, &thread_function, NULL);

        // Main game loop
        while (1) {
            if (game == GAME_END) {
                pthread_join(thread_id, NULL); // Wait for the thread to finish

                printf("\n\n Final score : %ld ", point);
                printf("\n\n Please enter your name : ");
                scanf("%s%*c", temp_result.name);
                temp_result.point = point;

                if (temp_result.point >= best_point)
                    best_point = temp_result.point;


                ptime = time(NULL); 
                t = localtime(&ptime); 

                temp_result.year = t->tm_year + 1900;
                temp_result.month = t->tm_mon + 1;
                temp_result.day = t->tm_mday;
                temp_result.hour = t->tm_hour;
                temp_result.min = t->tm_min;

                fp = fopen("result", "ab");
                fseek(fp, 1, SEEK_END);
                fwrite(&temp_result, sizeof(struct result), 1, fp);
                fclose(fp);

                x = 3, y = 0;
                point = 0;
                tetrisplayMusicFlag = 0;               // 노래 중지
                pthread_join(musicThread, NULL);

                return 1;
            }
        }
    }

    return 0;
}

int display_tetris_table(void)
{
    int i, j;
    char(*block_pointer)[4][4][4] = NULL;

    switch (next_block_number)
    {
    case I_BLOCK:	block_pointer = &i_block;
        break;
    case T_BLOCK:	block_pointer = &t_block;
        break;
    case S_BLOCK:  block_pointer = &s_block;
        break;
    case Z_BLOCK: 	block_pointer = &z_block;
        break;
    case L_BLOCK: 	block_pointer = &l_block;
        break;
    case J_BLOCK: 	block_pointer = &j_block;
        break;
    case O_BLOCK:	block_pointer = &o_block;
        break;
    }

    system("clear");
    printf("\n\n Next Block\n");

    for (i = 0; i < 4; i++)
    {
        printf("\n");
        for (j = 0; j < 4; j++)
        {
            if ((*block_pointer)[0][i][j] == 1)
                printf("#");
            else if ((*block_pointer)[0][i][j] == 0)
                printf(" ");
        }
    }
    for (i = 2; i < 21; i++)
    {
        printf("\t");
        for (j = 0; j < 15; j++)
        {
            if (j == 0 || j == 14 || (i == 20 && (j > 1 || j < 14)))
            {
                printf("@");
            }
            else if (tetris_table[i][j] == 1)
                printf("#");
            else if (tetris_table[i][j] == 0)
                printf(" ");
        }
        printf("\n");
    }

    return 0;
}

int init_tetris_table(void)
{
    int i = 0, j = 0;

    for (i = 0; i < 20; i++)
        for (j = 1; j < 14; j++)
            tetris_table[i][j] = 0;

    for (i = 0; i < 21; i++)
    {
        tetris_table[i][0] = 1;
        tetris_table[i][14] = 1;
    }

    for (j = 1; j < 14; j++)
        tetris_table[20][j] = 1;

    return 0;
}

int getch1(void)
{
    char   ch;
    int   error;
    static struct termios Otty, Ntty;

    fflush(stdout);
    tcgetattr(0, &Otty);
    Ntty = Otty;
    Ntty.c_iflag = 0;
    Ntty.c_oflag = 0;
    Ntty.c_lflag &= ~ICANON;
#if 1
    Ntty.c_lflag &= ~ECHO;
#else
    Ntty.c_lflag |= ECHO;
#endif
    Ntty.c_cc[VMIN] = CCHAR;
    Ntty.c_cc[VTIME] = CTIME;

#if 1
#define FLAG TCSAFLUSH
#else
#define FLAG TCSANOW
#endif

    if (0 == (error = tcsetattr(0, FLAG, &Ntty)))
    {
        error = read(0, &ch, 1);
        error += tcsetattr(0, FLAG, &Otty);
    }

    return (error == 1 ? (int)ch : -1);
}

int refresh1(int signum)
{
    static int downcount = 0;
    static int setcount = 0;
    static long speedcount = 0;
    static int countrange = 5;
    static int firststart = 0;
    static int pasued;
    char ch;
    int keykey = 0;

    srand((unsigned)time(NULL));

    if (firststart == 0)
    {
        block_number = rand() % 7;
        if (firststart == 0)
            firststart++;
    }

    system("clear");
    printf("\n Score: %ld | Speed: %d | hihgest score: %d", point, countrange, best_point);

    display_tetris_table();
    fflush(stdout);
    check_one_line();
    
        if (!pasued) {
            if (downcount == countrange - 1)
            {
                point += 1;
                move_block(DOWN);
            }

            if (speedcount == 499)
            {
                if (countrange != 1)
                    countrange--;
            }

            downcount++;
            downcount %= countrange;
            speedcount++;
            speedcount %= 500;
        }
        if (x == 3 && y == 0)
        {
            if (collision_test(LEFT) || collision_test(RIGHT) || collision_test(DOWN) || collision_test(UP))
            {
                printf("\n Game End! \n");
                downcount = 0;
                setcount = 0;
                speedcount = 0;
                countrange = 5;
                firststart = 0;
                game = GAME_END;
            }
        }

        if (collision_test(DOWN))
        {
            if (setcount == 9)
            {
                block_number = next_block_number;
                next_block_number = rand() % 7;
                block_state = 0;
                x = 5;
                y = 0;
            }
            setcount++;
            setcount %= 10;
        }
    readKetetris();
     ch = getch1();

    switch (ch)
    {
    case 81:
    case 113:
        pasued = !pasued; // 추가: P 버튼을 누를 때 일시정지 상태 변경
        break;
    }
    return 0;
}

int move_block(int command)
{
    int i, j;
    int newx, newy;
    int oldx, oldy;
    int old_block_state;
    char(*block_pointer)[4][4][4] = NULL;

    newx = x;
    newy = y;

    old_block_state = block_state;

    if (collision_test(command) == 0)
    {
        switch (command)
        {
        case	LEFT:	newx--;
            break;
        case	RIGHT:	newx++;
            break;
        case	DOWN:	newy++;
            break;
        case UP:	block_state++;
            block_state %= 4;
            break;
        }
    }
    else
    {
        return 1;
    }

    switch (block_number)
    {
    case I_BLOCK:	block_pointer = &i_block;
        break;
    case T_BLOCK:	block_pointer = &t_block;
        break;
    case S_BLOCK:  block_pointer = &s_block;
        break;
    case Z_BLOCK: 	block_pointer = &z_block;
        break;
    case L_BLOCK: 	block_pointer = &l_block;
        break;
    case J_BLOCK: 	block_pointer = &j_block;
        break;
    case O_BLOCK:	block_pointer = &o_block;
        break;
    }

    for (i = 0, oldy = y; i < 4; i++, oldy++)
    {
        for (j = 0, oldx = x; j < 4; j++, oldx++)
        {
            if (oldx > 0 && oldx < 14 && oldy < 20 && oldy > 0)
                if ((*block_pointer)[old_block_state][i][j] == 1)
                    tetris_table[oldy][oldx] = 0;

        }
    }

    x = newx;
    y = newy;

    for (i = 0, newy = y; i < 4; i++, newy++)
    {
        for (j = 0, newx = x; j < 4; j++, newx++)
        {
            if (newx > 0 && newx < 14 && newy < 20 && newy > 0)
                if ((*block_pointer)[block_state][i][j] == 1)
                    tetris_table[newy][newx] = (*block_pointer)[block_state][i][j];
        }
    }

    return 0;
}

int collision_test(int command)
{
    int i, j;
    int tempx, tempy;
    int oldx, oldy;
    int temp_block_state;
    char(*block_pointer)[4][4][4];
    char temp_tetris_table[21][15];

    oldx = tempx = x;
    oldy = tempy = y;
    temp_block_state = block_state;

    switch (command)
    {
    case	LEFT:	tempx--;
        break;
    case	RIGHT:	tempx++;
        break;
    case	DOWN:	tempy++;
        break;
    case UP: temp_block_state++;
        temp_block_state %= 4;
        break;
    }

    switch (block_number)
    {
    case I_BLOCK:	block_pointer = &i_block;
        break;
    case T_BLOCK:	block_pointer = &t_block;
        break;
    case S_BLOCK:  block_pointer = &s_block;
        break;
    case Z_BLOCK: 	block_pointer = &z_block;
        break;
    case L_BLOCK: 	block_pointer = &l_block;
        break;
    case J_BLOCK: 	block_pointer = &j_block;
        break;
    case O_BLOCK:	block_pointer = &o_block;
        break;
    }

    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 15; j++)
        {
            temp_tetris_table[i][j] = tetris_table[i][j];
        }
    }

    for (i = 0, oldy = y; i < 4; i++, oldy++)
    {
        for (j = 0, oldx = x; j < 4; j++, oldx++)
        {
            if (oldx > 0 && oldx < 14 && oldy < 20 && oldy > 0)
            {
                if ((*block_pointer)[block_state][i][j] == 1)
                    temp_tetris_table[oldy][oldx] = 0;
            }
        }
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {

            if (temp_tetris_table[tempy + i][tempx + j] == 1 && (*block_pointer)[temp_block_state][i][j] == 1)
                return 1;
        }
    }

    return 0;
}

int drop(void)
{
    while (!collision_test(DOWN))
        move_block(DOWN);

    return 0;
}

int check_one_line(void)
{
    int i, j;
    int ti, tj;
    int line_hole;

    for (i = 19; i > 0; i--)
    {
        line_hole = 0;
        for (j = 1; j < 14; j++)
        {
            if (tetris_table[i][j] == 0)
            {
                line_hole = 1;
            }
        }

        if (line_hole == 0)
        {
            point += 1000;
            for (ti = i; ti > 0; ti--)
            {
                for (tj = 0; tj < 14; tj++)
                {
                    tetris_table[ti][tj] = tetris_table[ti - 1][tj];
                }
            }
        }
    }

    return 0;
}

int print_result(void)
{
    FILE* fp = NULL;
    char ch = 1;

    fp = fopen("result", "rb");

    if (fp == NULL)
        return 0;

    system("clear");

    printf("\n\t\t\t\tText Tetris");
    printf("\n\t\t\t\t Game Stats\n\n");
    printf("\n\t\tName\t\tScore\t   Date\t\t Time");

    while (1)
    {
        fread(&temp_result, sizeof(struct result), 1, fp);
        if (!feof(fp))
        {
            printf("\n\t========================================================");
            printf("\n\t\t%s\n\t\t\t\t%ld\t %d.%d.%d.  |  %d:%d\n", temp_result.name, temp_result.point, temp_result.year, temp_result.month, temp_result.day, temp_result.hour, temp_result.min);
        }
        else
        {
            break;
        }
    }

    fclose(fp);

    printf("\n\n\tBack to the game menu : M");
    while (1)
    {
        ch = getch1();
        if (ch == 77 || ch == 109)
            break;
    }
    return 0;

}
//**************************************************************
//*****************미로 게임 코드시작********************************
//**************************************************************

// 미로 게임 배경 음악 노트
int mazeGameMelody[] = {
    262, 330, 392, 523, 392, 330, 262, // A 섹션
    440, 349, 440, 523, 392, 330, 262, // B 섹션
    392, 330, 262, 196, 330, 392, 262  // C 섹션
};

// 각 노트의 지속 시간 (밀리초 단위)
int noteDurations[] = {
    250, 250, 250, 250, 250, 250, 500, // A 섹션
    250, 250, 250, 250, 250, 250, 500, // B 섹션
    250, 250, 250, 250, 250, 250, 500  // C 섹션
};

void *playMusic(void *arg)
{
    int melodyLength = sizeof(mazeGameMelody) / sizeof(int);
    while (playMusicFlag)
    {
        for (int i = 0; i < melodyLength; i++)
        {
            if (!playMusicFlag)
                break; // 노래 중지 확인
            int note = mazeGameMelody[i];
            int duration = noteDurations[i];
            playNote(note, duration);
        }
    }
    return NULL;
}

/*  Carve the maze starting at x, y. */
void CarveMaze(char *maze, int width, int height, int x, int y)
{

    int x1, y1;
    int x2, y2;
    int dx, dy;
    int dir, count;

    dir = rand() % 4;
    count = 0;
    while (count < 4)
    {
        dx = 0;
        dy = 0;
        switch (dir)
        {
        case 0:
            dx = 1;
            break;
        case 1:
            dy = 1;
            break;
        case 2:
            dx = -1;
            break;
        default:
            dy = -1;
            break;
        }
        x1 = x + dx;
        y1 = y + dy;
        x2 = x1 + dx;
        y2 = y1 + dy;
        if (x2 > 0 && x2 < width && y2 > 0 && y2 < height && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1)
        {
            maze[y1 * width + x1] = 0;
            maze[y2 * width + x2] = 0;
            x = x2;
            y = y2;
            dir = rand() % 4;
            count = 0;
        }
        else
        {
            dir = (dir + 1) % 4;
            count += 1;
        }
    }
}

/* Generate maze in matrix maze with size width, height. */
void GenerateMaze(char *maze, int width, int height)
{

    int x, y;

    /* Initialize the maze. */
    for (x = 0; x < width * height; x++)
    {
        maze[x] = 1;
    }
    maze[1 * width + 1] = 0;

    /* Seed the random number generator. */
    srand(time(0));

    /* Carve the maze. */
    for (y = 1; y < height; y += 2)
    {
        for (x = 1; x < width; x += 2)
        {
            CarveMaze(maze, width, height, x, y);
        }
    }

    /* Set up the entry and exit. */
    maze[0 * width + 1] = 0;
    maze[(height - 1) * width + (width - 2)] = 0;
}

/* Solve the maze. */
void SolveMaze(char *maze, int width, int height)
{

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
    while (x != width - 2 || y != height - 2)
    {
        dx = 0;
        dy = 0;
        switch (dir)
        {
        case 0:
            dx = 1;
            break;
        case 1:
            dy = 1;
            break;
        case 2:
            dx = -1;
            break;
        default:
            dy = -1;
            break;
        }
        if ((forward && maze[(y + dy) * width + (x + dx)] == 0) || (!forward && maze[(y + dy) * width + (x + dx)] == 2))
        {
            maze[y * width + x] = forward ? 2 : 3;
            x += dx;
            y += dy;
            forward = 1;
            count = 0;
            dir = 0;
        }
        else
        {
            dir = (dir + 1) % 4;
            count += 1;
            if (count > 3)
            {
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
void ShowMaze(const char* maze, int width, int height, int player_x, int player_y) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminal_width = w.ws_col;

    char line[width * 2 + 1]; // 각 줄에 대한 버퍼

    for (int y = 0; y <= height; y++) {
        int lineIdx = 0; // 현재 줄의 인덱스
        for (int x = 0; x <= width; x++) {
            if (x == player_x && y == player_y) {
                line[lineIdx++] = 'P';
                line[lineIdx++] = ' ';
            }
            else if (maze[y * width + x] == 1 || y == height) {
                line[lineIdx++] = '[';
                line[lineIdx++] = ']';
            }
            else {
                line[lineIdx++] = ' ';
                line[lineIdx++] = ' ';
            }
        }
        line[lineIdx] = '\0'; // 문자열 종료
        printCentered(line, terminal_width);
    }
}


// 사용자의 이동을 처리하는 함수
int MovePlayer(char *maze, int width, int height, int *player_x, int *player_y, char direction)
{
    int dx = 0, dy = 0;
    switch (direction)
    {
    case UP:
        dy = -1;
        break; // 위로 이동
    case DOWN:
        dy = 1;
        break; // 아래로 이동
    case LEFT:
        dx = -1;
        break; // 왼쪽으로 이동
    case RIGHT:
        dx = 1;
        break; // 오른쪽으로 이동
    }
    int new_x = *player_x + dx;
    int new_y = *player_y + dy;
    if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height && maze[new_y * width + new_x] == 0)
    {
        *player_x = new_x;
        *player_y = new_y;
        if (*player_x == width - 1 && *player_y == height - 1)
        {
            return 2; // 미로 탈출 성공
        }

        return 1; // 이동 성공
    }
    return 0; // 이동 실패
}

int Maze_Start(int music)
{
    int i;
    int width = 10;  // 너비
    int height = 10; // 높이
    int level = 1;   // 레벨
    int score = 0;   // 점수
    int state = 0;
    int inkey;
    int nKeypadstate;
    int stop = 0;
    int playerId = 1;  // playerId를 선언하고 초기값을 할당합니다.

    char *maze = malloc(width * height);
    // 음악 재생 스레드 생성
    pthread_t musicThread;
    
    if (maze == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }
    softToneCreate(BUZZER_PIN);

    if (music == 1)
    {
        pthread_create(&musicThread, NULL, playMusic, NULL);
    }
    GenerateMaze(maze, width, height);
    int player_x = 1, player_y = 1; // 플레이어 시작 위치

    time_t startTime = time(NULL);
    time_t endTime = startTime + TIMEOUT_DURATION;
    int timeoutMessageDisplayed = 0;

    char input;
    do
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int terminal_width = w.ws_col;

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

        printFormattedCentered(terminal_width,"Level: %d  Score: %d  Time : %d\n", level, score,(int)(endTime - time(NULL)-3));
        ShowMaze(maze, width, height, player_x, player_y);
        nKeypadstate = KeypadRead();
        inkey = -1; // 입력키에 -1 저장
        inkey = readKey();

        if (inkey == -1)
        {
            for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
            {
                if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                {
                    inkey = i + 5;
                }
            }
        }
        if (inkey != -1 && inkey == 5)
        {
            stop = 1;
            int cursor = 1;

            while (stop == 1)
            {
                nKeypadstate = KeypadRead();
                inkey = -1; // 입력키에 -1 저장
                inkey = readKey();

                if (inkey == -1)
                {
                    for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
                    {
                        if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                        {
                            inkey = i + 5;
                        }
                    }
                }
                system("clear");
                if (cursor == 1)
                {
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
                    printf("\n");
                    printf("\n");
                    printf("\n");
                    printCentered("1. 게임 계속 하기 O", terminal_width);
                    printCentered("2. 게임 종료", terminal_width);
                }
                else if (cursor == 2)
                {
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
                    printf("\n");
                    printf("\n");
                    printf("\n");
                    printCentered("1. 게임 계속 하기", terminal_width);
                    printCentered("2. 게임 종료 O", terminal_width);
                }

                if (inkey != -1 && inkey == CLICK && cursor == 1)
                {
                    stop = 0;
                    cursor = 1;
                }
                else if (inkey != -1 && inkey == CLICK && cursor == 2)
                {
                    state = 3;
                    stop = 0;
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
                delay(250);
            }
        }
         if (!timeoutMessageDisplayed && time(NULL) >= endTime - MESSAGE_DURATION)
        {
            // Display timeout message for 3 seconds
            pthread_t timeoutMessageThread;
            pthread_create(&timeoutMessageThread, NULL, displayTimeoutMessage, NULL);
            pthread_join(timeoutMessageThread, NULL);

            UpdateRanking(playerId,score, level);
            state =3;
            timeoutMessageDisplayed = 1;
        }
        if (state != 3)
            state = MovePlayer(maze, width, height, &player_x, &player_y, inkey);
        if (state == 2)
        {
            printf("\t\t\t\t\t\t\t\t\t미로 탈출 성공!\n");
            delay(1000);
            score += 100;
            level++;
            width += 2;
            height += 2;
            endTime= (time_t)(endTime+11);

            GenerateMaze(maze, width, height);
            player_x = 1;
            player_y = 1;
        
         // 순위 업데이트
        printf("\n\n\t\t\t\t\t\t\t\t\t=== Maze Rankings ===\n");
        displayMazeRankings();

        }

        delay(250);
    } while (state != 3); // 'q'를 누를 때까지 반복
    
        if (time(NULL) >= endTime)
    {
        printf("\n\n\t\t\t\t\t\t\t\t\t=== Time's up! ===\n");
        // Prompt user for name and record ranking
        UpdateRanking(playerId, score, level);
        
    }
    playMusicFlag = 0;               // 노래 중지
    pthread_join(musicThread, NULL); // 스레드 종료 대기


    free(maze);
    return 0;
}

 

// Function to display rankings in descending order
void displayMazeRankings()
{
    // Open the file for reading
    FILE *file = fopen("ranking.txt", "r");

    if (file != NULL)
    {
        RankingEntry entries[100]; // Assume a maximum of 100 entries
        int numEntries = 0;

        // Read entries from the file
        while (fscanf(file, "%d %d %d", &entries[numEntries].playerId, &entries[numEntries].level, &entries[numEntries].score) == 3)
        {
            printf("Read: Player %d  - Score: %d  Level: %d\n", entries[numEntries].playerId, entries[numEntries].level, entries[numEntries].score);
            numEntries++;
        }
              // 터미널 크기를 얻습니다.
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int terminal_width = w.ws_col;
        printCentered("\n\t\t\t\t\t\t\t\t\t - 순위- ",terminal_width);
        // Close the file
        fclose(file);

        // Sort entries using qsort
        qsort(entries, numEntries, sizeof(RankingEntry), compareRankingEntries);

        // Display sorted entries
        int rank = 1;
        for (int i = 0; i < numEntries; i++) {
            printFormattedCentered(terminal_width, " %d. \x1b[6;30;%dmPlayer %d - Score: %d  Level: %d\x1b[0m\n", rank, 47, entries[i].playerId, entries[i].level, entries[i].score);
            rank++;
        }
    }
    else
    {
        printf("Error opening file.\n");
    }
}

// Function to record maze ranking
void recordMazeRanking(const int playerId, int level, int score)
{
    // Open the file for reading
    FILE *file = fopen("ranking.txt", "r");

    if (file != NULL)
    {
        RankingEntry entries[100]; // Assume a maximum of 100 entries
        int numEntries = 0;

        // Read entries from the file
        while (fscanf(file, "\t\t\t %d\t %d\t %d\t", &entries[numEntries].playerId, &entries[numEntries].level, &entries[numEntries].score) == 3)
        {
            numEntries++;
        }

        // Close the file
        fclose(file);

        // Check if the player's entry already exists
        for (int i = 0; i < numEntries; i++)
        {
            if (entries[i].playerId == playerId)
            {
                printf("Player %d already has an entry in the rankings.\n", playerId);
                return; // Exit the function without appending duplicate data
            }
        }

        // Open the file for appending
        file = fopen("ranking.txt", "a");

        if (file != NULL)
        {
            // Record the player's ID, level, and score
            fprintf(file, "\t\t\t %d\t %d\t %d\t\n", playerId, score, level);

            // Close the file
            fclose(file);
        }
        else
        {
            printf("Error opening file for appending.\n");
        }
    }
    else
    {
        printf("Error opening file for reading.\n");
    }
}

void UpdateRanking(const int playerId, int level, int score)
{
    // Open the file for reading
    FILE *file = fopen("ranking.txt", "r+");

    if (file != NULL)
    {
        RankingEntry entries[100]; // Assume a maximum of 100 entries
        int numEntries = 0;

        // Read entries from the file
        while (fscanf(file, "\t\t\t %d\t %d\t %d\t", &entries[numEntries].playerId, &entries[numEntries].level, &entries[numEntries].score) == 3)
        {
            numEntries++;
        }

        // Check if the player's entry already exists
        int existingEntryIndex = -1;
        for (int i = 0; i < numEntries; i++)
        {
            if (entries[i].playerId == playerId)
            {
                existingEntryIndex = i;
                break;
            }
        }
        fclose(file);
        if (existingEntryIndex != -1)
        {
            // Update existing entry
            entries[existingEntryIndex].level = level;
            entries[existingEntryIndex].score = score;
        }
        else
        {
            // Add the new entry to the array
            entries[numEntries].playerId = playerId;
            entries[numEntries].level = level;
            entries[numEntries].score = score;
            numEntries++;
        }

        // Sort entries using qsort
        qsort(entries, numEntries, sizeof(RankingEntry), compareRankingEntries);

        // Open the file for writing (clearing existing content)
        file = fopen("ranking.txt", "w");

        if (file != NULL)
        {
            // Record sorted entries to the file
            for (int i = 0; i < numEntries; i++)
            {
                fprintf(file, "\t\t\t %d\t %d\t %d\t\n", entries[i].playerId, entries[i].level, entries[i].score);
            }

            // Close the file
            fclose(file);

            // Display sorted entries
            int rank = 1;
            for (int i = 0; i < numEntries; i++)
            {
                printf("%d. Player %d - Score: %d Level: %d\n", rank, entries[i].playerId, entries[i].level, entries[i].score);
                rank++;
            }
        }
        else
        {
            printf("Error opening file for writing.\n");
        }
    }
    else
    {
        printf("Error opening file for reading.\n");
    }
}


//**************************************************************
//*****************미로 게임 코드끝********************************
//**************************************************************

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

    Buzzer_Init();
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
        inkey = -1; // 입력키에 -1 저장
        inkey = readKey();

        if (inkey == -1)
        {
            for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
            {
                if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                {
                    inkey = i + 5;
                }
            }
        }

        if (inkey != -1 && sound == 1)
        {
            Change_FREQ(440); // 440Hz 주파수 소리 재생
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
        printCentered("\t\t\t\t  □□□■■□□□□□■□□□□■■□□□□■□□□□□□■□□□□□■□□□□□□□□□□□□□□■□□■□□□□□□□■□□□□□□□□□□□■□■□□■■■■■■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■■■■■■□□□■□□□□■■□□□□■□□□■■■■■■■□□■□□□□□■■■■■■□□□■□□■□□□□□□□■□□□□■■■■■□□■□■□□■■□□■■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■■□□■■□□□■□□□□■■□□□□■□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□□■□□■□■□□■□□□□■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■□□□□■□□□■□□□□■■■□□□■■■□□■■■■■□□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■□□■□■□□■□□□□■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■□□□□■□□□■□□□■■■■■□□■□□□■■□□□■■□□■■■□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■□□■■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■□□□□■□□□■□■■■□□■■■□■□□□■■□□□■■□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□□■■■■■□■□□■■■■■■□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■■□□□■□□□■□■■□□□□□□□■□□□□■■■■■□□□■□□□□□■□□□□■□□□■□□■□□□□□□□■□□□□□□■■□□□■□■□□□□□□□□□□□■□", terminal_width);
        printCentered("\t\t\t\t  □■■□□■■□□□■□□□□□■■■■■□□□□□□■■■□□□□■□□□□□■□□□□■□□□■□□■□□□□■■□■□□□□□■■■□□□■□■□□□□■■■■■■■■□", terminal_width);
        printCentered("\t\t\t\t  □□■■■■■□□□■□□□□■■□□□■■□□□□□■□□□□□□■□□□□□■■■■■■□□□■□□■■■■■■■□■□□□□■■□□□□□■□■□□□□■□□□□□□■□", terminal_width);
        printCentered("\t\t\t\t  □□□□□□□□□□■□□□□■□□□□□■□□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□■□□□□□□■□■□□□□■□□□□□□■□", terminal_width);
        printCentered("\t\t\t\t  □□□□□□□□□□■□□□□■■□□□■■□□□□□■□□□□□□□□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■□□□□□□■□", terminal_width);
        printCentered("\t\t\t\t  □□□□□□□□□□■□□□□□■■■■■□□□□□□■■■■■■■■□□□□□□□□□□□□□□■□□□□□□□□□□■□□□□□□□□□□□■□■□□□□■■■■■■■■□", terminal_width);

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
                 game = GAME_START;
                 game_start();
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
        else if (select == 2)
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
                print_result();
                // 테트리스 게임 순위 출력

                select = 0;
                cursor = 1;
            }
            if (inkey != -1 && inkey == CLICK && cursor == 1)
            {
               print_result();
                // 테트리스 게임 순위 출력

                select = 0;
                cursor = 1;
            }
            else if (inkey != -1 && inkey == CLICK && cursor == 2)
            {
                // 미로 찾기 게임 순위 출력
                printf("\e[1;1H\e[2J");
                    displayMazeRankings(); // Display rankings before recording new score
                while (1)
                {
                    nKeypadstate = KeypadRead();
                    inkey = -1; // 입력키에 -1 저장
                    inkey = readKey();

                    if (inkey == -1)
                    {
                        for (i = 0; i < MAX_KEY_BT_NUM; i++) // 반복문 for, 매개변수 i, 0부터 시작하여 KEYPAD 버튼 최대 개수인 12 미만까지 +1, 아래 코드 반복
                        {
                            if (nKeypadstate & (1 << i)) // 조건문 if, 현재 i의 값을 1 Left Shift하여 nKeypadstate 값과 AND연산했을때(버튼의 위치 탐색)
                            {
                                inkey = i + 5;
                            }
                        }
                    }
                    if (inkey != -1 && inkey == 6)
                    {
                        select = 0;
                        cursor = 1;
                        break;
                    }
                    if (inkey != -1 && sound == 1)
                    {
                        Change_FREQ(440); // 440Hz 주파수 소리 재생
                        usleep(100000);   // 0.1초 대기 (100000 마이크로초)
                        STOP_FREQ();      // 소리 정지
                    }
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
            if(inkey!=-1&&inkey==CLICK&&cursor==2){
                select=0;
                cursor=1;
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
        else if (select == 4)
        {
            break;
        }

        // 일정 시간 동안 대기합니다 (예: 0.25초).
        delay(250);
    }

    return 0;
}
