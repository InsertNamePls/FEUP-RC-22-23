#include "utils.h"

int main(int argc, char *argv[]){
    //PROGRAM STEPS
    //1-Parse arguments
    if(argc != 2){
        printf("[ERROR] Invalid argument!\n");
        printf("*** Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    Arguments args;
    if(parse_arguments(&args, argv[1]) == -1){
        printf("[ERROR] Invalid argument!\n");
        printf("*** Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);    
    }   
    
    printf("Arguments: \n");
    printf("User: %s\n", args.user);
    printf("Password: %s\n", args.password);
    printf("Hostname: %s\n", args.host);
    printf("Path: %s\n", args.urlPath);

    //2-Create socket and connect to server

    // Get IP with hostname
    char ip_address[16];
    int port = 21;
    strcpy(ip_address, getIP(args.host));
    
    // Create and connect socket
    int sockfd = create_socket();
    connect_socket(sockfd, ip_address, port);

    // Just some feedback
    char *buffer = (char *)malloc(MAX_LENGTH*sizeof(char)); 
    size_t size = MAX_LENGTH;
    read_from_socket(sockfd, buffer, size);

    //3-Login with user and password
    //4-Enter passive mode
    //5-Calculate port
    //7-Send the file to client with the given port
    //8-Close sockets
    if(close_connection(sockfd) < 0){
        printf("[ERROR] Couldn't close the connection.\n");
        exit(-1);
    }

    free(buffer);
    //printf("FTP Download Protocol in the makings, come back soon!\n");

    return 0;
}