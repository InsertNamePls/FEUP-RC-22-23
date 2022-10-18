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
/*
//converts string to hex
void string2hexString(unsigned char* contentC, unsigned char* contentH){
  int len = strlen(contentC);
  for(int i = 0; i<len; i++){
     sprintf(contentH+i*2, "%02X", contentC[i]);
  }
}

//verifies the existence of file
int exists(const char *fname){
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}

//reads contents of file and inserts it into char array
void read_app(unsigned char *encoded,char *input_path){
  char buff;
  int counter = 0;
  FILE *file;
  if(exists(input_path) == TRUE) {
    file = fopen(input_path,"r");

    do {
      buff = fgetc(file);
      if(buff != EOF){
        encoded[counter] = buff;
        counter++;
      }

    } while(buff != EOF);
    encoded[counter] = '\0';

    fclose(file);
  }
  else{
    fprintf(fl,"[ERROR]  File does not exist\n");
    exit(-1);
  }
}

int packageData(unsigned char* data,unsigned char* packagedContent,int current, int numSeq, int beg,int interval) {
  //header --start--
  packagedContent[current] = 0x01; //
  packagedContent[current+1] = (numSeq >> 8) & 0xFF; // N (1)
  packagedContent[current+2] = numSeq & 0xFF; // N (2)
  packagedContent[current+3] = ((beg+interval) >> 8) & 0xFF; // L2 (1)
  packagedContent[current+4] = (beg+interval) & 0xFF; //L2 (2)
  packagedContent[current+5] = (beg >> 8) & 0xFF; // L1 (1)
  packagedContent[current+6] = beg & 0xFF; // L1 (2)
  current+=7;

  for(int i = beg; i < (beg+interval); i++){
    packagedContent[current+(i-beg)] = data[i];
  }
  current+= interval;

  return current;
}
int hex_to_int(char c){
        int first = c / 16 - 3;
        int second = c % 16;
        int result = first*10 + second;
        if(result > 9) result--;
        return result;
}

int hex_to_ascii(char c, char d){
        int high = hex_to_int(c) * 16;
        int low = hex_to_int(d);
        return high+low;
}


int package_app(unsigned char *contentH,unsigned char *packagedContent,char *input_path, int fd){

  FILE *file;
  int start = 0, counter = 0,beg = 0,buf;
  unsigned char temp;
  int written=0;
  int set = llopen_em(fd);
  if(set < 0) end(written,-1);


  //Convert input path to hex
  int input_path_Hlen = strlen(input_path) * 2 + 1;
  char *input_path_H = (char*)malloc(input_path_Hlen*sizeof(char));
  string2hexString(input_path,input_path_H);

  //control frame - nome do ficheiro
  packagedContent[0] = 0x02;
  packagedContent[1] = 0x01;
  packagedContent[2] = strlen(input_path_H) & 0xFF;
  //printf("%X\n", packagedContent[2]);

  start += 3;
  for(int i = start; i < start+strlen(input_path_H); i++){
    packagedContent[i] = input_path_H[i-start];
  }
  start += strlen(input_path_H) + 1;
  written += llwrite(fd,packagedContent,&start);
  start = 0;

  free(input_path_H);


  //control frame - tamanho do ficheiro
  packagedContent[start] = 0x00;
  packagedContent[start+1] = 0x04;
  packagedContent[start+2] = (strlen(contentH) >> 16) & 0xFF;
  packagedContent[start+3] = (strlen(contentH) >> 8) & 0xFF;
  packagedContent[start+4] = strlen(contentH) & 0xFF;
  start+=5;
  written += llwrite(fd,packagedContent,&start);
  start = 0;

  int remaining = strlen(contentH) - beg;

  while (remaining > PACK_SIZE) {
    int start = 0;
    start = packageData(contentH,packagedContent,start,counter,beg,PACK_SIZE);
    counter++;
    beg += PACK_SIZE - 1;
    remaining = strlen(contentH) - beg;
    written += llwrite(fd,packagedContent,&start);
  }
  buf = 0;
  start = packageData(contentH,packagedContent,buf,counter,beg,remaining);
  packagedContent[start] = 0x03;
  written += llwrite(fd,packagedContent,&start);

  return written;

}
*/

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
    if (index < sizeof(dataSection)-1)
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

      //need byte stuffing
     // llwrite(packet);
      index = 2;
      
    }
  }
  fclose(pengu);
}

unsigned char *enconde()
{
  // add header and control variables
  // byte stuff and ready to send
  return NULL;
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

  1 - Connection
  2 - Open File
  3 - Send Data
  4 - Close Connection


  */
}
