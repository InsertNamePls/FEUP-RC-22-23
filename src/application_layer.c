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
#define MAX_FIELD_SIZE 255

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

    //sleep(3);
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
  if(fsizeLen > MAX_FIELD_SIZE){
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
  if(fnameLen > MAX_FIELD_SIZE){
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

  if (cons.role == LlTx)
  {
    // Transmitter starts transfering data:
    // First a control packet is sent
    // Then data packets are created, sent by llwrite and validated by reader
    FILE *pengu = getFile(filename);
    if (pengu != NULL){
      struct stat file_info;
      stat(filename, &file_info);

      unsigned char controlPacket[PAYLOAD];
      int ctrlPacketSize = createControlPacket(controlPacket, START, file_info, filename);
      int bytes_written;
      /*printf("[CONTROL PACKET] ");
      for(int i=0;i<ctrlPacketSize;i++){
        printf("%x ", controlPacket[i]);
      }
      printf("\n");*/
      if(bytes_written = llwrite(controlPacket, ctrlPacketSize) < 0) {
        printf("[ERROR] Control packet start couldn't be sent.\n");
        printf("[LOG] Aborting data transfer.\n");
        llclose(0);
        return;
      }
      
      apWrite(pengu);

      ctrlPacketSize = createControlPacket(controlPacket, END, file_info);
      if(bytes_written = llwrite(controlPacket, ctrlPacketSize) < 0) {
        printf("[ERROR] Control packet end couldn't be sent.\n");
        printf("[LOG] Aborting data transfer.\n");
        llclose(0);
        return;
      }
    }
    llclose(1);
  }
  else if (cons.role == LlRx)
  {
    //Reader starts reading file data with llread:
    //Data packets are read, validated and written on file in case of success
    printf("[LOG] Reader Ready.\n");

    int bytes_read;
    unsigned char buf[PAYLOAD];
    // Read start control packet
    if((bytes_read = llread(buf)) < 0) {
      printf("[ERROR] Couldn't read control packet.\n");
      llclose(0);
      return;
    } else {
      
    }
    
    FILE *file = fopen(filename, "wb");

    //verify control packet start
    bytes_read = llread(buf);

    int i=0;
    for(i = 0; i < 10968/PAYLOAD+2;i++){

      /*printf("BYTES READ: %d \n", bytes_read);
      printf("[FINAL PACKET #%d] ", i);
      for(int j=0;j<bytes_read;j++)
        printf("%x ", buf[j]);
      printf("\n");*/
      bytes_read = llread(buf);
      if(bytes_read >= 0) 
        fwrite(buf, sizeof(unsigned char), bytes_read, file);
      else printf("[ERROR] Llread didn't work as expected.\n");
    }
    
     fclose(file);
  }
}
