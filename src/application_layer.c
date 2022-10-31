// Application layer protocol implementation
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>

#include "application_layer.h"
#include "link_layer.h"

#define PAYLOAD 600 // mover isto para configs.h
#define DATA 0x01
#define START 0x02
#define END 0x03

LinkLayerRole getRole(const char *role)
{
  if (!strcmp(role, "tx"))
    return LlTx;
  else if (!strcmp(role, "rx"))
    return LlRx;
  else
  {
    printf("[ERROR] Invalid Role!\n");

    return (LinkLayerRole)NULL;
  }
}

LinkLayer getParams(const char *serialPort, const char *role, int baudRate,
                    int nTries, int timeout)
{
  LinkLayer connectionParams;
  strcpy(connectionParams.serialPort, serialPort);
  connectionParams.role = getRole(role);
  connectionParams.baudRate = baudRate;
  connectionParams.nRetransmissions = nTries;
  connectionParams.timeout = timeout;
  return connectionParams;
}

FILE *getFile(const char *filename)
{
  FILE *file = NULL;
  file = fopen(filename, "rb");
  return file;
}

void apWrite(FILE *pengu)
{
  unsigned char buffer[PAYLOAD];
  int bytesRead = 0;
  int totalBytesRead = 0;
  int bytesWritten = 0;
  int i=0;
  while ((bytesRead = fread(buffer, 1, PAYLOAD, pengu)) > 0)
  {
    /*printf("[INITIAL PACKET #%d] ", i);
      for(int j=0;j<bytesRead;j++)
        printf("%x ", buffer[j]);
    printf("\n");*/

    unsigned char dataPacket[bytesRead + 4];
    dataPacket[0] = DATA; // control field
    dataPacket[1] = i % 255; // sequence number
    dataPacket[2] = bytesRead / 256; // number of octects (divisor)
    dataPacket[3] = bytesRead % 256; // number of octets (remainder)

    memcpy(dataPacket + 4, buffer, bytesRead);
    if((bytesWritten = llwrite(dataPacket, bytesRead + 4)) == -1){
      printf("[ERROR] Llwrite did not work as expected.\n");
      printf("[LOG] Aborting data transfer.\n");
      return;
    }

    totalBytesRead += bytesRead;
    i++;
  }
  fclose(pengu);
}

int createControlPacket(unsigned char* packet, unsigned char type, struct stat fInfo){
  packet[0] = type;     // Control field
  packet[1] = 0         // T field for file size
  packet[2] = fInfo.st_size // L field for file size
  //packet[3] =         // V field for file size
  //packet[4+file_size] = 1         // T field for filename
  //int len = 
  //packet[5+file_size] = len       // L field for filename
  //packet[6+file_size] =         // V field for filename

  return -1;
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
  printf("[PROTOCOL START]\n");
  LinkLayer cons = getParams(serialPort, role, baudRate, nTries, timeout);
  if (llopen(cons) < 0)
  {
    perror("[ERROR] Connection Opening Error!\n");
    return;
  }

  if (cons.role == LlTx)
  {
    // Transmitter starts transfering data:
    // First a control packet is sent
    // Then data packets are created, sent by llwrite and validated by reader
    FILE *pengu = getFile(filename);
    if (pengu != NULL){
      struct stat file_info;
      stat(filename, &file_info);

      unsigned char* controlPacket;
      int ctrlPacketSize = createControlPacket(controlPacket, START, file_info);
      //int bytes_written = llwrite(controlPacket, ctrlPacketSize);
      
      apWrite(pengu);

      ctrlPacketSize = createControlPacket(controlPacket, END, file_info);
      //bytes_written = llwrite(controlPacket, ctrlPacketSize);
    }
    llclose(1);
  }
  else if (cons.role == LlRx)
  {
    //Reader starts reading file data with llread:
    //Data packets are read, validated and written on file in case of success
    printf("[LOG] Reader Ready.\n");

    unsigned char buf[PAYLOAD];
    FILE *file = fopen(filename, "wb");

    int bytes_read;

    //verify control packet start
    //bytes_read = llread(buf);

    int i=0;
    while((bytes_read = llread(buf)) > 0){

      /*printf("BYTES READ: %d \n", bytes_read);
      printf("[FINAL PACKET #%d] ", i);
      for(int j=0;j<bytes_read;j++)
        printf("%x ", buf[j]);
      printf("\n");*/

      fwrite(buf, sizeof(unsigned char), bytes_read, file);
      i++;
    }
    
     fclose(file);
  }
}
