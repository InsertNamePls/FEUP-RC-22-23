#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

/* Flag Configs*/
#define F 0x7E
#define A_W 0x03
#define A_R 0x01
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define I_0 0x00
#define I_1 0x40
#define RR_0 0x05
#define RR_1 0x85
#define REJ_0 0x01
#define REJ_1 0x81

#define S1 0x7D
#define S2 0x5E
#define S3 0x5D

#define BCC1_SET A_W ^ SET
#define BCC1_UA_R A_W ^ UA
#define BCC1_UA_W A_R ^ UA
#define BCC_I0 A_W ^ I_0
#define BCC_I1 A_W ^ I_1 
#define BCC1_DISC_W A_W ^ DISC
#define BCC1_DISC_R A_R ^ DISC
#define BCC1_RR0 A_W ^ RR_0
#define BCC1_RR1 A_W ^ RR_1
#define BCC1_REJ0 A_W ^ REJ_0
#define BCC1_REJ1 A_W ^ REJ_1

/* SET State Machine Configuration*/
#define START 1
#define FLAG_RCV 2
#define A_RCV 3
#define C_RCV 4
#define BCC_OK 5
#define STOP 6
#define DISC_RCV 7
#define I_RCV 8
#define RR_RCV 9
#define REJ_RCV 10
#define BCC_I_OK 11
#define BCC_DISC_OK 12
#define BCC_RR_OK 13
#define BCC_REJ_OK 14
#define A

/*Generic Frames*/
unsigned char _SET[5] = {F, A_W, SET, BCC1_SET, F};
unsigned char _UA_R[5] = {F, A_W, UA, BCC1_UA_R, F};
unsigned char _DISC_W[5] = {F, A_W, DISC, BCC1_DISC_W, F};
unsigned char _DISC_R[5] = {F, A_R, DISC, BCC1_DISC_R, F};
unsigned char _RR_0[5] = {F, A_W, RR_0, BCC1_RR0, F};
unsigned char _REJ_0[5] = {F, A_W, REJ_0, BCC1_REJ0, F};
unsigned char _RR_1[5] = {F, A_W, RR_1, BCC1_RR1, F};
unsigned char _REJ_1[5] = {F, A_W, REJ_1, BCC1_REJ1, F};
unsigned char _UA_W[5] = {F, A_R, UA, BCC1_UA_W, F};

/*Global Settings*/
#define FALSE 0
#define TRUE 1
#define PAYLOAD 600

/*Global Variables*/
int fd;
int alarmEnabled = FALSE;
int connectionEnabled = FALSE;
int alarmCount = 0;
int curSeqNum = 0;
int disconnect = FALSE;
int transmitingData = FALSE;
int next_IFrame = FALSE;
struct termios oldtio;
struct termios newtio;
