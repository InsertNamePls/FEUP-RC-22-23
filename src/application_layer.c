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
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"

struct stat file_info;
int file_size;

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
  // connectionParams.serialPort = serialPort; ---- View this later ? problem?
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
#define PAYLOAD 600 // mover isto para configs.h

void apWrite(FILE *pengu)
{
  unsigned char buffer[PAYLOAD];
  unsigned char packet[PAYLOAD + 3];
  int bytesRead = 0;
  int totalBytesRead = 0;
  while ((bytesRead = fread(buffer, 1, PAYLOAD, pengu)) > 0)
  {
    packet[0] = 0; /// TODO control field
    packet[1] = 1; // sequence number
    packet[2] = 2; // number of octects
    memcpy(packet + 3, buffer, sizeof(packet));
    llwrite(packet, sizeof(packet));
    totalBytesRead += bytesRead;
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
    FILE *pengu = getFile(filename);
    if (pengu != NULL){
      stat(filename, &file_info);
      file_size = file_info.st_size;

      printf("FILESIZE: %d\n", file_size);
      
      apWrite(pengu);
    }
    llclose(1);
  }
  else if (cons.role == LlRx)
  {
    printf("[LOG] Reader Ready.\n");

    unsigned char buf[PAYLOAD];
    FILE *file = fopen(filename, "wb");
    //printf("FILESEIZE/PAYLOAD = %d\n", file_size/PAYLOAD);
    
    for(int i=0; i<18; i++){
      int bytes_read = llread(buf);
          //if (bytes_read == 5) break;
      fwrite(buf, sizeof(unsigned char), sizeof(buf), file);
    }
    
     fclose(file);
  }

  // llclose(0);
}
