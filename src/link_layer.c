// Link layer protocol implementation
#include "configs.h"
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

/*Aux Functions*/
void setupPorts(LinkLayer connectionParameters)
{
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        perror(connectionParameters.serialPort);
        exit(-1);
    }

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
}

int next_State(unsigned char recieved, unsigned char expected, int next, int *current)
{
    if (recieved == expected)
    {
        *current = next;
        return TRUE;
    }
    return FALSE;
}

void closePorts()
{
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("[LOG] Communication Terminated.\n");

    close(fd);
}

int compare_response(unsigned char *A, unsigned char *B, int size)
{
    if (memcmp(A, B, size) == 0)
        return TRUE;
    return FALSE;
}

int read_UA()
{
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        /* mudar isto para chamadas a funcao LL read*/
        unsigned char buf[2];
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        // printf("Char Read: %x\n", buf[0]);
        /*------------------------------------------*/
        if (bytes_read != 0)
        {
            switch (state)
            {
            case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                    state = START;
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, UA, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_UA_R, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case BCC_OK:
                if (!next_State(char_received, F, STOP, &state))
                {
                    state = START;
                }
                else
                {
                    connected = TRUE;
                    printf("[LOG] Reader Connected.\n");
                }
                break;
            default:
                break;
            }
        } 
        if (alarmEnabled == FALSE) 
        {
            break;
        }
    }
    
    return connected;
}

int read_SET()
{
    unsigned char buf[2];
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        if (bytes_read != 0)
        {
            switch (state)
            {
            case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                    state = START;
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, SET, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_SET, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case BCC_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else
                {
                    printf("[LOG] Writter Connected.\n");
                    connected = TRUE;
                    write(fd, _UA_R, 5);
                }
                break;
            default:
                break;
            }
        }
        if(alarmEnabled == FALSE)
        {
            break;
        }
    }
    return connected;
}

int read_IFrame(){
    unsigned char buf[2];
    int state = START;
    int connection = FALSE;
    while(state != STOP){
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        if(bytes_read != 0){
            switch(state){
                case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                    state = START;
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, I_0, I_RCV, &state) || next_State(char_received, I_1, I_RCV, &state) ||  next_State(char_received, DISC, DISC_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case I_RCV:
                if(!(next_State(char_received, BCC_I0, BCC_I_OK, &state) || next_State(char_received, BCC_I1, BCC_I_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case DISC_RCV:
                if(!(next_State(char_received, BCC1_DISC_W, I_RCV, &state) || next_State(char_received, BCC1_DISC_R, I_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case BCC_I_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = BCC_I_OK;
                else
                {
                    printf("[LOG] Valid Information Frame.\n");
                    connection = TRUE;
                    printf("[LOG] Sending RR frame.\n");
                    if(curSeqNum == 0) write(fd, _RR_0, 5);
                    else if(curSeqNum == 1) write(fd, _RR_1, 5);
                    else printf("[ERROR] Invalid sequence number!\n");
                }
                break;
            case BCC_DISC_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else
                {
                    printf("[LOG] Read DISC frame.\n");
                    printf("[LOG] Sending response DISC frame.\n");
                    write(fd, _DISC_R, 5);
                }
                break;
            default:
                break;
            }
        }
    }
    return connection;
}

int read_IFrameRes(){
    unsigned char buf[2];
    int state = START;
    int connection = FALSE;
    while(state != STOP){
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        if(bytes_read != 0){
            switch(state){
                case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                    state = START;
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, RR_0, RR_RCV, &state) || next_State(char_received, RR_1, RR_RCV, &state) || next_State(char_received, REJ_0, REJ_RCV, &state) || next_State(char_received, REJ_1, REJ_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case RR_RCV:
                if(!(next_State(char_received, BCC1_RR0, BCC_RR_OK, &state) || next_State(char_received, BCC1_RR1, BCC_RR_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case REJ_RCV:
                if(!(next_State(char_received, BCC1_REJ0, BCC_REJ_OK, &state) || next_State(char_received, BCC1_REJ1, BCC_REJ_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case BCC_RR_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else
                {
                    printf("[LOG] Received RR.\n");
                    connection = TRUE;
                    //verify sequence number
                }
                break;
            case BCC_REJ_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else
                {
                    printf("[LOG] Received REJ.\n");
                    connection = TRUE;
                    //verify sequence number
                }
                break;
            default:
                break;
            }
        }
        else if (alarmEnabled == FALSE){
            break;
        }
    }
    return connection;
}

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
}

unsigned char getCvalue(){
    if(curSeqNum == 0) return I_0;
    else if(curSeqNum == 1) return I_1;
    return -1;
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    setupPorts(connectionParameters);

    if (connectionParameters.role == LlTx)
    {
        (void)signal(SIGALRM, alarmHandler);

        while (alarmCount < connectionParameters.nRetransmissions && connectionEnabled == FALSE)
        {
            // Start the alarm
            if (alarmEnabled == FALSE)
            {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;
            }

            // Send SET
            printf("Sending SET [%d]!\n", alarmCount);
            printf("[LOG] Initializing Communication.\n");
            write(fd, _SET, 5);

            sleep(1);

            // Read the UA and verify it
            if (read_UA() == TRUE)
                connectionEnabled = TRUE;
            else
            {
            printf("[ERROR] Connection Failed.\n");
            printf("[LOG] Retrying connection.\n");  
            } 
        }

        if (connectionEnabled == TRUE)
        {
            alarmEnabled = FALSE;
            alarmCount = 0;

            printf("[LOG] Waiting for data.\n");
        }
        else
        {
            printf("[LOG] Connection failed.\n");
            printf("[LOG] Exiting.\n");
            llclose(0);
            exit(-1);
        }
    }
    else
    {
        // receiver
        (void)signal(SIGALRM, alarmHandler);

         while (alarmCount < connectionParameters.nRetransmissions && connectionEnabled == FALSE)
        {
            // Start the alarm
            if (alarmEnabled == FALSE)
            {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;
            }

            // Read SET
            printf("Checking for SET [%d]!\n", alarmCount);
            printf("[LOG] Initializing Communication.\n");
            if (read_SET() == TRUE)
                connectionEnabled = TRUE;
            else
            {
            printf("[ERROR] Connection Failed.\n");
            printf("[LOG] Retrying connection.\n");  
            }
        }

        if (connectionEnabled == TRUE)
        {
            alarmEnabled = FALSE;
            alarmCount = 0;

            printf("[LOG] Waiting for data.\n");
        }
        else
        {
            printf("[LOG] Connection failed.\n");
            printf("[LOG] Exiting.\n");
            llclose(0);
            exit(-1);
        }
    }

    if (connectionEnabled == TRUE)
        return 1;
    printf("%d \n", connectionEnabled);
    return -1;
}

char calculateBCC2(unsigned char* packet, int packetsize){
    unsigned char bcc2 = 0x00;
    for(int i = 0; i < packetsize;i++){
        bcc2 = bcc2 ^ packet[i];
    }
    return bcc2;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    // Return variable
    int totalWrittenBytes = -1;

    // Stuffing frames to be sent
    unsigned char auxBuffer[bufSize*2];
    int auxBufferIndex=0;
    char bcc2 = calculateBCC2(buf,bufSize);
    for (int i = 0; i<bufSize;i++){
        if (buf[i] == F){ 
           auxBuffer[auxBufferIndex] = S1;
           auxBufferIndex++;
           auxBuffer[auxBufferIndex] = S2;
        }else if (buf[i] == S1){
            auxBuffer[auxBufferIndex] = S1;
            auxBufferIndex++;
            auxBuffer[auxBufferIndex] = S3;

        }else{
            auxBuffer[auxBufferIndex] = buf[i];
        }
        auxBufferIndex++;
    }

    // Preparing final packet
    int finalpacketSize = auxBufferIndex + 6;
    unsigned char finalPacket[finalpacketSize];

    finalPacket[0] = F;
    finalPacket[1] = A_W;
    finalPacket[2] = getCvalue();
    finalPacket[3] = finalPacket[1] ^ finalPacket[2];

    memcpy(finalPacket + 4, auxBuffer, auxBufferIndex);
    
    finalPacket[finalpacketSize -2] = bcc2; // need to stuff bcc2;
    finalPacket[finalpacketSize -1] = F;
    
    // DEBUG prints
    printf("\n\n###########################################################################\n\n");
    printf("[PACKET]");
    for (int i = 0; i < finalpacketSize; i++)
        printf("%x ", finalPacket[i]);
    printf("\n");
    
    (void)signal(SIGALRM, alarmHandler);
    connectionEnabled = FALSE;



    return totalWrittenBytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{ 
    int i = 0;
    unsigned char buf[2]; 
    unsigned char holder[PAYLOAD + 10];
    int realsize = 0;
    
    while ( i < PAYLOAD + 10) // mudar isto para state machine e meter para Flag
    {
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        if (bytes_read > 0)
        {
            holder[i] = char_received; 
            printf("data[%i]: %x \n ",i, char_received);
            i++;
        }
    }

    unsigned char aux[sizeof(holder)];
  
    for (int i = 0; i<sizeof(holder)-1;i++){
        if (holder[i] == S1 && holder[i+1] == S2){ 
           aux[realsize] = F;
           i++;
        }else if (holder[i] == S1 && holder[i+1] == S3){
            aux[realsize] = S1;
            i++;
        }else{
            aux[realsize] = holder[i];
        }
        realsize++;
    }

    /*
    check bbc1; fazer a verificacao de bbc1 e bbc2
    check bbc2; 
    */

    printf("Unstuffed size: %d\n", realsize);
    unsigned char data[PAYLOAD];
    for (int i = 7; i < realsize - 2; i++)
    {
        data[i - 7] = aux[i];
    }


    FILE *file = fopen("pengu.gif", "wb+");
    fwrite(data, sizeof(unsigned char), sizeof(data), file);
    fclose(file);
    return 1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    closePorts();
    return 1;
}
