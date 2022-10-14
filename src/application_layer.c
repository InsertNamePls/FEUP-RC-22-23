// Application layer protocol implementation

#include "application_layer.h"
#include "link_layer.h"

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    
    LinkLayer connectionParams;
  //  connectionParams.serialPort = serialPort; ---- View this later ? problem?
    strcpy(connectionParams.serialPort,serialPort);
    connectionParams.role = role;
    connectionParams.baudRate = baudRate;
    connectionParams.nRetransmissions = nTries;
    connectionParams.timeout = timeout;
    llopen(connectionParams);
    // TODO
}
