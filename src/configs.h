#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/* Flag Configs*/
#define F 0x7E
#define A_W 0x03
#define A_R 0x01
#define SET 0x03
#define DISC 0x0B
#define UA 0x07

#define BCC1_SET A_W ^ SET
#define BCC1_UA A_R ^ UA

/* SET State Machine Configuration*/
#define START 1
#define FLAG_RCV 2
#define A_RCV 3
#define C_RCV 4
#define BCC_OK 5
#define STOP 6

/*Generic Frames*/
unsigned char _SET[5] = {F, A_W, SET, BCC1_SET, F};
unsigned char _UA[5] = {F, A_R, UA, BCC1_UA, F};

/*Global Settings*/
#define FALSE 0
#define TRUE 1
#define PAYLOAD 600

/*Global Variables*/
int fd;
int alarmEnabled = FALSE;
int connectionEnabled = FALSE;
int alarmCount = 0;
struct termios oldtio;
struct termios newtio;
