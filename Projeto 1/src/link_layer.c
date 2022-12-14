// Link layer protocol implementation
#include "configs.h"
#include "link_layer.h"

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

LinkLayer ll;

/*Aux Functions*/
LinkLayer initLinkLayer(LinkLayer cons)
{
    LinkLayer ll;
    ll.baudRate = cons.baudRate;
    ll.nRetransmissions = cons.nRetransmissions;
    ll.role = cons.role;
    strcpy(ll.serialPort, cons.serialPort);
    ll.timeout = cons.timeout;

    return ll;
}

void setupPorts(LinkLayer connectionParameters)
{
    ll = initLinkLayer(connectionParameters);

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

int read_DISC_R()
{
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        unsigned char buf[2];
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
                if (!(next_State(char_received, A_R, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, DISC, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_DISC_R, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
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
                    printf("[LOG] Starting Disconnection.\n");
                }
                break;
            default:
                break;
            }
        }
        if (alarmEnabled == FALSE)
            break;
    }
    return connected;
}

int read_DISC_W()
{
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        unsigned char buf[2];
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
                if (!(next_State(char_received, DISC, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_DISC_W, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
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
                    printf("[LOG] Starting Disconnection.\n");
                }
                break;
            default:
                break;
            }
        }
        if (alarmEnabled == FALSE)
            break;
    }
    return connected;
}

int read_UA()
{
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        unsigned char buf[2];
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

int read_UA_W()
{
    int state = START;
    int connected = FALSE;
    while (state != STOP && connected == FALSE)
    {
        unsigned char buf[2];
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
                if (!(next_State(char_received, A_R, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, UA, C_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case C_RCV:
                if (!(next_State(char_received, BCC1_UA_W, BCC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
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
    }
    return connected;
}

int read_IFrame(unsigned char *buf, int *bufSize)
{
    int state = START;
    int frame_rcv = FALSE;
    int i = 0;
    while (state != STOP && frame_rcv == FALSE)
    {
        int bytes_read = read(fd, buf + i, 1);
        unsigned char char_received = buf[i];
        if (bytes_read > 0)
        {
            switch (state)
            {
            case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    i++;
                }
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    i++;
                }
                break;
            case A_RCV:
                if (!(next_State(char_received, I_0, I_RCV, &state) || next_State(char_received, I_1, I_RCV, &state) || next_State(char_received, DISC, DISC_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    i++;
                }
                break;
            case I_RCV:
                if (!(next_State(char_received, BCC_I0, BCC_I_OK, &state) || next_State(char_received, BCC_I1, BCC_I_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    i++;
                }
                break;
            case DISC_RCV:
                if (!(next_State(char_received, BCC1_DISC_W, BCC_DISC_OK, &state) || next_State(char_received, BCC1_DISC_R, BCC_DISC_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    i++;
                }
                break;
            case BCC_I_OK:
                if (!next_State(char_received, F, STOP, &state))
                {
                    state = BCC_I_OK;
                    i++;
                }
                else
                {
                    frame_rcv = TRUE;
                }
                break;
            case BCC_DISC_OK:
                if (!next_State(char_received, F, STOP, &state))
                {
                    state = START;
                    i = 0;
                }
                else
                {
                    transmitingData = FALSE;
                }
                break;
            default:
                break;
            }
            *bufSize += bytes_read;
        }
    }

    return frame_rcv;
}

int read_IFrameRes(int *totalBytes)
{
    unsigned char buf[2];
    int state = START;
    int frame_rcv = FALSE;
    // get a rcv_control_value to save the received frame control value and
    // to later check with the current sequence value
    int rcv_control_value;

    while (state != STOP && alarmEnabled == TRUE)
    {
        int bytes_read = read(fd, buf, 1);
        *totalBytes += bytes_read;
        unsigned char char_received = buf[0];
        if (bytes_read != 0)
        {
            switch (state)
            {
            case START:
                if (!next_State(char_received, F, FLAG_RCV, &state))
                {
                    state = START;
                }
                break;
            case FLAG_RCV:
                if (!(next_State(char_received, A_W, A_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case A_RCV:
                if (!(next_State(char_received, RR_0, RR_RCV, &state) || next_State(char_received, RR_1, RR_RCV, &state) || next_State(char_received, REJ_0, REJ_RCV, &state) || next_State(char_received, REJ_1, REJ_RCV, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                else if (char_received != F)
                {
                    if (char_received == RR_0 || char_received == REJ_0)
                        rcv_control_value = 0;
                    else
                        rcv_control_value = 1;
                }
                break;
            case RR_RCV:
                if (!(next_State(char_received, BCC1_RR0, BCC_RR_OK, &state) || next_State(char_received, BCC1_RR1, BCC_RR_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case REJ_RCV:
                if (!(next_State(char_received, BCC1_REJ0, BCC_REJ_OK, &state) || next_State(char_received, BCC1_REJ1, BCC_REJ_OK, &state) || next_State(char_received, F, FLAG_RCV, &state)))
                    state = START;
                break;
            case BCC_RR_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else if (rcv_control_value == (curSeqNum + 1) % 2)
                {
                    // If the control value received (in a RR) is the opposite of the I frame one then it can send another one
                    printf("[LOG] Received RR.\n");
                    frame_rcv = TRUE;
                    next_IFrame = TRUE;
                    curSeqNum = (curSeqNum + 1) % 2;
                }
                else
                {
                    // If the control value received is invalid then send the I frame again
                    printf("[ERROR] RR: Wrong sequence number, discarting frame.\n");
                    frame_rcv = TRUE;
                }
                break;
            case BCC_REJ_OK:
                if (!next_State(char_received, F, STOP, &state))
                    state = START;
                else if ((rcv_control_value) == curSeqNum)
                {
                    printf("[LOG] Received corresponding REJ.\n");
                    frame_rcv = TRUE;
                }
                else
                {
                    printf("[ERROR] REJ: Wrong sequence number, discarting frame.\n");
                    frame_rcv = TRUE;
                }
                break;
            default:
                break;
            }
        }
    }
    return frame_rcv;
}

void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("[LOG] Timeout!\n");
}

unsigned char getCvalue()
{
    if (curSeqNum == 0)
    {
        return I_0;
    }
    else if (curSeqNum == 1)
    {
        return I_1;
    }
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
        // Read SET
        printf("[LOG] Checking for SET\n");
        printf("[LOG] Initializing Communication.\n");
        if (read_SET() == TRUE)
            connectionEnabled = TRUE;
        else
        {
            printf("[ERROR] Connection Failed.\n");
        }
    }

    if (connectionEnabled == TRUE)
    {
        printf("[LOG] Waiting for data.\n");
        transmitingData = TRUE;
    }
    else
    {
        printf("[LOG] Connection failed.\n");
        printf("[LOG] Exiting.\n");
        llclose(0);
        exit(-1);
    }

    if (connectionEnabled == TRUE)
        return 1;
    printf("%d \n", connectionEnabled);
    return -1;
}

unsigned char calculateBCC2(const unsigned char *packet, int packetsize)
{
    unsigned char bcc2 = 0x00;
    for (int i = 0; i < packetsize; i++)
    {
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
    unsigned char auxBuffer[bufSize * 2];
    int auxBufferIndex = 0;
    unsigned char bcc2 = calculateBCC2(buf, bufSize);

    for (int i = 0; i < bufSize; i++)
    {
        if (buf[i] == F)
        {
            auxBuffer[auxBufferIndex] = S1;
            auxBufferIndex++;
            auxBuffer[auxBufferIndex] = S2;
        }
        else if (buf[i] == S1)
        {
            auxBuffer[auxBufferIndex] = S1;
            auxBufferIndex++;
            auxBuffer[auxBufferIndex] = S3;
        }
        else
        {
            auxBuffer[auxBufferIndex] = buf[i];
        }
        auxBufferIndex++;
    }

    // need to stuff bcc2
    if (bcc2 == F) {
        auxBuffer[auxBufferIndex] = S1;
        auxBufferIndex++;
        auxBuffer[auxBufferIndex] = S2;
    } else if (bcc2 == S1){
        auxBuffer[auxBufferIndex] = S1;
        auxBufferIndex++;
        auxBuffer[auxBufferIndex] = S3;
    } else {
        auxBuffer[auxBufferIndex] = bcc2;
    }
    auxBufferIndex++;

    // Preparing final packet for info frame
    int finalpacketSize = auxBufferIndex + 5;
    unsigned char finalPacket[finalpacketSize];
    // Assign Header
    finalPacket[0] = F;
    finalPacket[1] = A_W;
    finalPacket[2] = getCvalue();
    finalPacket[3] = finalPacket[1] ^ finalPacket[2];
    // Copy buffer data to packet
    memcpy(finalPacket + 4, auxBuffer, auxBufferIndex);
    // Complete packet with trailer
    finalPacket[finalpacketSize - 1] = F;

    next_IFrame = FALSE;

    (void)signal(SIGALRM, alarmHandler);

    while (alarmCount < ll.nRetransmissions && next_IFrame == FALSE)
    {
        // Call the alarm
        if (alarmEnabled == FALSE)
        {
            // Send the Information packet
            printf("[LOG] Sending packet. (Attempt #%d)\n", alarmCount + 1);
            printf("[LOG] Waiting for confirmation.\n");

            totalWrittenBytes = write(fd, finalPacket, finalpacketSize);

            alarm(ll.timeout);
            alarmEnabled = TRUE;
        }

        // Read response
        // Only leaves the sending frame loop once it has received a correct RR
        if (read_IFrameRes(&totalWrittenBytes) == TRUE)
        {
            // If it read a frame response and it's REJ
            if (next_IFrame == FALSE)
            {
                printf("[LOG] Invalid Info Frame.\n");
                printf("[LOG] Resending packet.\n");

                alarm(0);
                alarmEnabled = FALSE;
                alarmCount++;
            }
        }
        else
        {
            printf("[ERROR] Response not received.\n");
            printf("[LOG] Retrying connection.\n");
        }
    }
    if (alarmEnabled == TRUE)
    {
        alarmEnabled = FALSE;
        alarmCount = 0;
    }

    return totalWrittenBytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{
    unsigned char *holder = (unsigned char*)malloc(sizeof(unsigned char*)*(PAYLOAD+10));
    unsigned char aux[PAYLOAD + 10];

    int realsize = 0;
    int totalReadBytes = -1;

    // It can only return FALSE when DISC is found
    int frame_read = read_IFrame(holder, &totalReadBytes);
    if (totalReadBytes > -1)
    {
        //  get the value right since it started at -1 for error checking purposes
        totalReadBytes++;
        if (frame_read == FALSE)
        {
            printf("[LOG] Received a Disconnect Frame.\n");
            printf("[LOG] Sending response DISC frame.\n");

            write(fd, _DISC_R, 5);

            printf("[LOG] Reading UA.\n");
            read_UA_W();
            printf("[LOG] UA received, communication terminated.\n");
            closePorts();
        }
        else
        {
            // Start destuffing the holder
            for (int i = 0; i < totalReadBytes; i++)
            {
                if (holder[i] == S1 && holder[i + 1] == S2)
                {
                    aux[realsize] = F;
                    i++;
                }
                else if (holder[i] == S1 && holder[i + 1] == S3)    
                {
                    aux[realsize] = S1;
                    i++;
                }
                else
                {
                    aux[realsize] = holder[i];
                }
                realsize++;
            }

            unsigned char teste_[realsize];
            for (int i = 0; i < realsize; i++)
                teste_[i] = aux[i];

            // Get the bcc2 in the packet and calculate bcc2 in the packet
            unsigned char bcc2_rcv = teste_[realsize - 2];
            unsigned char bcc2aux[realsize - 6];
            for (int i = 0; i < realsize - 6; i++)
            {
                bcc2aux[i] = teste_[i + 4];
            }

            // Remove headers
            for (int i = 8; i < realsize - 2; i++)
            {
                packet[i - 8] = teste_[i];
            }

            // Validate Info frame and choose which packet to send
            // +4 to skip F, A, C and BCC1 and -6 to not go over and skip BCC2 and F
            if (bcc2_rcv == calculateBCC2(bcc2aux,realsize - 6))
            {
                printf("[LOG] Packet Read successfully.\n");
                printf("[LOG] Sending RR frame.\n");

                if (holder[2] == I_0)
                    write(fd, _RR_1, 5);
                else if (holder[2] == I_1)
                    write(fd, _RR_0, 5);
                else
                    printf("[ERROR] Invalid sequence number!\n");
            }
            else
            {
                printf("[LOG] Faulty Info Packet.\n");
                printf("[LOG] Sending REJ frame.\n");

                if (holder[2] == I_0)
                    write(fd, _REJ_0, 5);
                else if (holder[2] == I_1)
                    write(fd, _REJ_1, 5);
                else
                    printf("[ERROR] Invalid sequence number!\n");
            }
        }
    }

    free(holder);

    return realsize-10;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{
    printf("[LOG] Attempt to close ports.\n");
    int final = 0;
    (void)signal(SIGALRM, alarmHandler);

    alarm(0);

    while (alarmCount < ll.nRetransmissions && connectionEnabled == TRUE)
    {
        // Start the alarm
        if (alarmEnabled == FALSE)
        {
            alarm(ll.timeout);
            alarmEnabled = TRUE;

            write(fd, _DISC_W, 5);
            printf("[LOG] Sent DISC frame.\n");
        }

        if (read_DISC_R() == TRUE)
        {
            printf("[LOG] Read DISC frame, sending UA and closing ports.\n");
            final = write(fd, _UA_W, 5);
            connectionEnabled = FALSE;
            closePorts();
        }
    }
    if (alarmEnabled == TRUE)
    {
        alarmCount = 0;
        alarmEnabled = FALSE;
    }

    if(showStatistics == TRUE){
        printf("[STATISTICS] Time running: %f\n", duration);
    }

    if (final != 0)
    {
        return 1;
    }
    return -1;
}