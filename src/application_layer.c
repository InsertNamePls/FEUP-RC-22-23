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
    /*printf("[INITIAL PACKET #%d] ", i);
      for(int j=0;j<bytesRead;j++)
        printf("%x ", buffer[j]);
    printf("\n");*/

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
    // Data packets are created, sent by llwrite and validated by reader
    FILE *pengu = getFile(filename);
    if (pengu != NULL){
      stat(filename, &file_info);
      file_size = file_info.st_size;

      //printf("FILESIZE: %d\n", file_size);
      
      apWrite(pengu);
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
