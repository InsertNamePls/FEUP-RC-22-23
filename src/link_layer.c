// Link layer protocol implementation
#include "configs.h"
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

/*Aux Functions*/
void setupPorts(LinkLayer connectionParameters)
{
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
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
    newtio.c_cc[VMIN] = 0;
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
                if (!(next_State(char_received, A_R, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, UA, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_UA, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
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
        /* mudar isto para chamadas a funcao LL open*/
        int bytes_read = read(fd, buf, 1);
        unsigned char char_received = buf[0];
        // printf("char read: %x\n", buf[0]);
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
                    /*Mandar SET se connected prov melhorar isto com alguma funcao or smth*/
                    write(fd, _UA, 5);
                }
                break;
            default:
                break;
            }
        }
    }
    return connected;
}

void alarmTx(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;
    printf("[LOG] Initializing Communication.\n");
    write(fd, _SET, 5);

    if (read_UA() == TRUE)
    {
        connectionEnabled = TRUE;
    }
    else
    {
        printf("[ERROR] Connection Failed.\n");
        printf("[LOG] Retrying connection.\n");
    }
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{
    setupPorts(connectionParameters);

    if (connectionParameters.role == LlTx)
    {
        (void)signal(SIGALRM, alarmTx);

        while (alarmCount < connectionParameters.nRetransmissions && connectionEnabled == FALSE)
        {
            if (alarmEnabled == FALSE)
            {
                alarm(connectionParameters.timeout);
                alarmEnabled = TRUE;
                // printf("Sending SET [%d]!\n", alarmCount);
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
            exit(-1);
        }
    }
    else
    {
        // receiver
        //(void)signal(SIGALRM, alarmRx);
        connectionEnabled = read_SET();
    }

    if (connectionEnabled == TRUE)
        return 1;
    printf("%d \n", connectionEnabled);
    return -1;
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{

    int finalpacketSize = bufSize + 6;
    unsigned char finalPacket[finalpacketSize];
    finalPacket[0] = F;
    finalPacket[1] = A_W;
    finalPacket[2] = 3; // NEED TO SEE C CALCULATE THIS
    finalPacket[3] = finalPacket[1] ^ finalPacket[2];

    memcpy(finalPacket + 4, buf, finalpacketSize);

    finalPacket[bufSize + 4] = 9; // NEED TO SEE BBC2 CALCULATE THIS
    finalPacket[bufSize + 5] = F;

    printf("[PACKET]");
    for (int i = 0; i < finalpacketSize; i++)
        printf("%x", finalPacket[i]);
    printf("\n");

    write(fd, finalPacket, finalpacketSize);

    return 0;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    return read(fd, packet, PAYLOAD);
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    closePorts();
    return 1;
}
