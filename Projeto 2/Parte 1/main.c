#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

int main(int argc, char *argv[]){
    //PROGRAM STEPS
    //1-Parse arguments
    if(argc != 2){
        printf("[ERROR] Invalid argument!\n");
        printf("*** Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }
    Arguments args;
    if(parse_arguments(&args, argv[1]) == -1){
        printf("[ERROR] Invalid argument!\n");
        printf("*** Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);    
    }   
    
    printf("Arguments: \n");
    printf("User: %s\n", args.user);
    printf("Password: %s\n", args.password);
    printf("Hostname: %s\n", args.host);
    printf("Path: %s\n", args.urlPath);

    //2-Create socket and connect to server
    //3-Login with user and password
    //4-Enter passive mode
    //5-Calculate port
    //7-Send the file to client with the given port
    //8-Close sockets

    //printf("FTP Download Protocol in the makings, come back soon!\n");

    return 0;
}