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
#include <time.h>

#include "application_layer.h"
#include "link_layer.h"

struct stat file_info;
int file_size;
#define PAYLOAD 600 // mover isto para configs.h

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
    unsigned char dataPacket[bytesRead + 4];
    dataPacket[0] = 1; // control field
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

int getBytesLength(int bytes){
  int len, i = 1;
  for(len = 0;i < bytes;len++){
    i *= 255;
  }

  return len;
}

// Return size of control packet or -1 in case of error
int createControlPacket(unsigned char* packet, unsigned char type, struct stat fInfo, char* filename){
  packet[0] = type;            // Control field
  packet[1] = 0x00;            // T field for file size

  // Get octets from filesize
  int filesize = fInfo.st_size;
  int fsizeLen = getBytesLength(filesize);
  if(fsizeLen > PAYLOAD){
    printf("[ERROR] File size cannot fit a byte.\n");
    return -1;
  }
  packet[2] = fsizeLen;
  
  // Save bytes on packet
  packet[3] = (unsigned)filesize & 0xFF; 
  for(int i = 1, j=8; i < fsizeLen; j*=2, i++){
    packet[3+i] = (unsigned)filesize >> j & 0xFF;
  }

  packet[3+fsizeLen] = 0x01;                       // T field for filename
  int fnameLen = strlen(filename);
  if(fnameLen > PAYLOAD){
    printf("[ERROR] File size cannot fit a byte.\n");
    return -1;
  }
  packet[4+fsizeLen] = fnameLen;                    // L field for filename
  for(int i=0;i<fnameLen;i++){
    packet[5+fsizeLen+i] = filename[i];
  }

  int finalsize = 5 + fsizeLen + fnameLen; 
  if(finalsize <= PAYLOAD) return finalsize;
  else {
    printf("[ERROR] Control packet exceeds maximum size.\n");
    return -1;
  }
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

  clock_t start, end;
  start = clock();

  if (cons.role == LlTx)
  {
    // Transmitter starts transfering data:
    // Data packets are created, sent by llwrite and validated by reader
    FILE *pengu = getFile(filename);
    if (pengu != NULL){
      stat(filename, &file_info);
      file_size = file_info.st_size;
      
      apWrite(pengu);
    }

    end = clock();
    duration = ((float)end - start)/CLOCKS_PER_SEC;

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
    while((bytes_read = llread(buf)) > 0){
      fwrite(buf, sizeof(unsigned char), bytes_read, file);
    }
    
     fclose(file);
  }
}
