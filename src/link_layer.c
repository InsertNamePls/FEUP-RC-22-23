// Link layer protocol implementation

#include "link_layer.h"
#include "configs.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

void setupPorts(LinkLayer connectionParameters){
    fd = open(connectionParameters.serialPort,O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }
    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }
    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; 
    newtio.c_cc[VMIN] = 0; 
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
}

void closePorts(){
    // Restore the old port settings
   /* if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("Closing Connection!\n");
    close(fd);
    */
}

int read_UA(){
    return FALSE;
}

int read_SET(){
    return FALSE;
}

void alarmTx(int signal){
    alarmEnabled = FALSE;
    alarmCount++;

   
    llwrite(_SET,5);
     
   if(read_UA() == TRUE){
        connectionEnabled = TRUE;
    }else{
        printf("Connection Failed! Retrying connection.\n");
    }

}

void alarmRx(int signal){
    
}


////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{

    setupPorts(connectionParameters);

    if(connectionParameters.role == LlTx){ 
        // transmitter
        (void)signal(SIGALRM, alarmTx);

        while (alarmCount < connectionParameters.nRetransmissions &&  connectionEnabled == FALSE)
        {
            if (alarmEnabled == FALSE)
            {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;
                printf("Sending SET [%d]!\n",alarmCount);
            }
        }

        if(connectionEnabled == TRUE){
            printf("Connection Established!\n");
            printf("Waiting for more data.\n");
            alarmEnabled = FALSE;
            alarmCount = 0;
        }
        else{
            printf("Connection failed.\n");
            printf("Exiting.\n");
            exit(-1);
        }

    }else{
        // receiver
        (void)signal(SIGALRM, alarmRx);
    }
    return 1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    write(fd, buf, bufSize);

    printf("[1] : %x \n",buf[0]);
    printf("[2] : %x \n",buf[1]);
    printf("[3] : %x \n",buf[2]);
    printf("[4] : %x \n",buf[3]);
    printf("[5] : %x \n",buf[4]);

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    // TODO

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    closePorts();
    return 1;
}
