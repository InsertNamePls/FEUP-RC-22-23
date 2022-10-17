// Link layer protocol implementation

#include "link_layer.h"
#include "configs.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

void setupPorts(LinkLayer connectionParameters){

}

void closePorts(){
    // Restore the old port settings
   /* 
    */
}

int compare_response(unsigned char* A,unsigned char* B,int size){
    if (memcmp(A,B,size)==0) return TRUE;
    return FALSE;
}

int read_UA(){
    unsigned char holder[5];
    llread(holder);
    printf("Printing read data:\n");
    printf("[1] : %x \n",holder[0]);
    printf("[2] : %x \n",holder[1]);
    printf("[3] : %x \n",holder[2]);
    printf("[4] : %x \n",holder[3]);
    printf("[5] : %x \n",holder[4]);
    return (compare_response(_UA,holder,sizeof(_UA)));
}

int next_State(unsigned char recieved,unsigned char expected, int next, int *current){
    if(recieved == expected){
        *current = next;
        return TRUE;
    }
    return FALSE;
}
    /*int x = 0;
    unsigned char holder[5];
    while(x < 10){
        llread(holder);
        llwrite(_UA,5);
        x++;
    }*/
int read_SET(){
    unsigned char buf[2];
    int state = START;
    int connected = FALSE;
    while(state != STOP && connected == FALSE){
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        printf("char read: %x\n",buf[0]);
        if(bytes_read != 0){
            switch(state){
                case START:
                    if(!next_State(char_received,F,FLAG_RCV,&state))
                        state = START;
                    break;
                case FLAG_RCV:
                    if(!(next_State(char_received,A_W,A_RCV,&state) || next_State(char_received,F,FLAG_RCV,&state)))
                        state = START;
                    break;
                case A_RCV:
                    if(!(next_State(char_received,SET,C_RCV,&state) || next_State(char_received,F,FLAG_RCV,&state)))
                        state = START;
                    break;
                case C_RCV:
                    if(!(next_State(char_received,BCC1_SET,BCC_OK,&state) || next_State(char_received,F,FLAG_RCV,&state)))
                        state = START;
                    break;
                case BCC_OK:
                    if(!next_State(char_received,F,STOP,&state))
                        state = START;
                    else{
                        connected = TRUE;
                    }
                    break;
                default:
                    break;
            }
        }      
    }
    return connected;
}

void alarmTx(int signal){
    alarmEnabled = FALSE;
    alarmCount++;

    llwrite(_SET,5);
     
    if(read_UA() == TRUE){
        printf("UA received!\n");
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

   // setupPorts(connectionParameters);

    /* Move this to aux functions*/

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
    newtio.c_cc[VMIN] = 1; 
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    /*-------------------*/

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
            alarmEnabled = FALSE;
            alarmCount = 0;
            printf("Connection Established!\n");
            printf("Waiting for more data.\n");
        }
        else{
            printf("Connection failed.\n");
            printf("Exiting.\n");
            exit(-1);
        }

    }else{
        // receiver
        (void)signal(SIGALRM, alarmRx);
        read_SET();
    }
    
    
    /*Move this to LL close*/
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("Closing Connection!\n");
    close(fd);
    
    /*-------------------*/
    return -1;
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
    return read(fd, packet, sizeof(packet));
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    closePorts();
    return 1;
}
