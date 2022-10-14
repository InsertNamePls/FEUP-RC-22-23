// Application layer protocol implementation
#include <string.h>
#include "application_layer.h"
#include "link_layer.h"

LinkLayerRole getRole(const char* role){
  if(!strcmp(role, "tx")) return LlTx;
  else if(!strcmp(role, "rx")) return LlRx;
  else {
    perror("Invalid Role!\n");
    return (LinkLayerRole) NULL;
  }
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {

    LinkLayer connectionParams;
    // connectionParams.serialPort = serialPort; ---- View this later ? problem?
    strcpy(connectionParams.serialPort,serialPort);
    connectionParams.role = getRole(role);
    connectionParams.baudRate = baudRate;
    connectionParams.nRetransmissions = nTries;
    connectionParams.timeout = timeout;

    //Open Connection
    if(llopen(connectionParams) > 0){
      perror("Connection Opening Error!\n");
      return;
    }

}
