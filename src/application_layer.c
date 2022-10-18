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

void printArray(unsigned int *array)
{
  for (int i = 0; i < sizeof(array); i++)
  {
    printf("%d", array[i]);
  }
  printf("\n");
}

void apWrite(FILE *pengu)
{
  unsigned char buffer[2];
  unsigned int dataSection[11];
  unsigned int packet[32];
  size_t bytesRead = 0;

  packet[0] = 0; // control field
  packet[1] = 1; // sequence number
  packet[2] = 2; // number of octects

  int index = 2;
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), pengu)) > 0)
  {
    if (index < sizeof(dataSection) - 1)
    {
      dataSection[index] = buffer[0];
      index++;
    }
    else
    {
      packet[0] = 0; // control field
      packet[1] = 1; // sequence number
      packet[2] = 2; // number of octects
      packet[sizeof(dataSection)] = buffer[0];
      printArray(packet);

      // need byte stuffing
      // llwrite(packet);
      index = 2;
    }
  }
  fclose(pengu);
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
  LinkLayer cons = getParams(serialPort, role, baudRate, nTries, timeout);
  if (llopen(cons) < 0)
  {
    perror("Connection Opening Error!\n");
    return;
  }

  if (cons.role == LlTx)
  {
    FILE *pengu = getFile(filename);
    if (pengu != NULL)
      apWrite(pengu);

    /*
    Read file data
    process file data
    send file data to reader via LLwrite
    */
  }
  else if (cons.role == LlRx)
  {
    printf("Ready to read data\n");
  }

  /*see this later*/

  /*

  1 - Connection X
  2 - Open File X
  3 - Send Data X
  4 - Close Connection


  */

 llclose();

 
}
