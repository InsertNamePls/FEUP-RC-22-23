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
    printf("[LOG] Connecting to %s:%d\n", ip_address,port);
    
    // Create and connect socket
    int sockfd = create_socket();
    connect_socket(sockfd, ip_address, port);

    // Just some feedback
    char *buffer = (char *)malloc(MAX_LENGTH*sizeof(char)); 
    size_t size = MAX_LENGTH;
    read_from_socket(sockfd, buffer, size);

    //3-Login with user and password
    send_credentials(sockfd, args.user, args.password);
    //4-Enter passive mode
    enter_passive_mode(sockfd);
    read_from_socket(sockfd, buffer, size);
    
    //5-Calculate port
    int new_port = get_new_port(buffer);
    if(new_port < 0){
        printf("[ERROR] Invalid port read from passive mode.\n");
        exit(-1);
    }
    //7-Send the file to client with the given port
    // Create receiver socket and connect
    int rcv_sockfd = create_socket();
    printf("[LOG] Connecting to %s:%d\n", ip_address, new_port);
    connect_socket(rcv_sockfd, ip_address, new_port);

    // Send file from control socket
    send_file(sockfd, args.urlPath);
    read_from_socket(sockfd, buffer, size);
    // Save to new file
    char filename[MAX_LENGTH];
    getFilename(filename, args.urlPath);
    FILE *f = fopen(filename, "w");
    save_to_file(rcv_sockfd, buffer, size, f);

    //8-Close sockets
    if(close_connection(sockfd) < 0){
        printf("[ERROR] Couldn't close the control socket connection.\n");
        exit(-1);
    }
    if(close_connection(rcv_sockfd) < 0){
        printf("[ERROR] Couldn't close the client socket connection.\n");
        exit(-1);
    }

    free(buffer);

    return 0;
}