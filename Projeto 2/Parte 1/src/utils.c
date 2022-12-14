#include "utils.h"

int parse_arguments(Arguments *args, char *input){
    // Parse beggining
    if(strncmp(input, "ftp://", 6) != 0){
        //printf("[ERROR] The input given must start with \"ftp://\"\n");
        return -1;
    }

    // Parse user and password if they exist
    if(strchr(input+6, '@') != NULL){
        if(sscanf(input, "ftp://%[^:]:%[^@]@%[^/]/%s", args->user, args->password, args->host, args->urlPath) != 4){
            printf("[ERROR] Couldn't parse user and pass\n");
            return -1;
        }
        /*printf("Arguments: \n");
        printf("User: %s\n", args->user);
        printf("Password: %s\n", args->password);
        printf("Hostname: %s\n", args->host);
        printf("Path: %s\n", args->urlPath);*/

        return 1;
    }
    else {
        if(sscanf(input, "ftp://%[^/]/%s", args->host, args->urlPath) != 2){
            printf("[ERROR] Couldn't parse host and path\n");
            return -1;
        } 

        strcpy(args->user, "anonymous");
        strcpy(args->password, "pass");

        /*printf("Arguments: \n");
        printf("Hostname: %s\n", args->host);
        printf("Path: %s\n", args->urlPath);*/

        return 1;
    }

    return -1;
}

int create_socket(){
    int sockfd; 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    return sockfd;
}

char* getIP(char* hostname){
    struct hostent *h;

    if ((h = gethostbyname(hostname)) == NULL) {
        printf("[ERROR] Couldn't get IP address.\n");
        exit(-1);
    }

    return inet_ntoa(*((struct in_addr *)h->h_addr));
}

void connect_socket(int sockfd, char* ip, int port){
    struct sockaddr_in server_addr;
    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        printf("[ERROR] Couldn't connect socket.\n");
        exit(-1);
    }
}

void read_from_socket(int sockfd, char* buffer, size_t size){
    FILE *fp = fdopen(sockfd, "r");
    printf("[LOG] From Socket #%d: \n", sockfd);
    do {
        buffer = fgets(buffer, size, fp);
        printf("%s", buffer);
    } while (!('1' <= buffer[0] && buffer[0] <= '5') || buffer[3] != ' ');

    if(buffer[0] == '5' && buffer[0] == '4'){
        printf("[ERROR] Failed action.\n");
    }
}

void send_credentials(int sockfd, char* user, char* password){
    printf("[LOG] Sending user to socket.\n"); 
    char buffer[MAX_LENGTH];
    char command[MAX_LENGTH]; 
    sprintf(command, "user %s\n", user);
    size_t bytes;
    if ((bytes = write(sockfd, command, 6+strlen(user))) <= 0){
        printf("[ERROR] Couldn't write to socket.\n");
        exit(-1);
    }
    read_from_socket(sockfd, buffer, MAX_LENGTH);
    //printf("Written %ld bytes\n", bytes);

    printf("[LOG] Sending password to socket.\n");
    sprintf(command, "pass %s\n", password);
    if ((bytes = write(sockfd, command, 6+strlen(password))) <= 0){
        printf("[ERROR] Couldn't write to socket.\n");
        exit(-1);
    }
    read_from_socket(sockfd, buffer, MAX_LENGTH);
    //printf("Written %ld bytes\n", bytes);
}

void enter_passive_mode(int sockfd){
    size_t bytes;
    printf("[LOG] Sending pasv to socket.\n");
    if ((bytes = write(sockfd, "pasv\n", 5)) <= 0){
        printf("[ERROR] Couldn't write to socket.\n");
        exit(-1);
    }
}

int get_new_port(char* buffer){
    int lb, rb;
    int ip_addr[4];
    if(sscanf(buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &ip_addr[0], &ip_addr[1], &ip_addr[2], &ip_addr[3], &lb, &rb) == 6){
        return lb*256+rb;
    }

    return -1;
}

void send_file(int sockfd, char* path){
    size_t bytes;
    char command[MAX_LENGTH]; 
    printf("[LOG] Sending file...\n");
    
    sprintf(command, "retr %s\n", path);
    if ((bytes = write(sockfd, command, strlen(command))) <= 0){
        printf("[ERROR] Couldn't write to socket.\n");
        exit(-1);
    }
    //printf("Written %ld bytes\n",  bytes);
}

void getFilename(char* filename, char* path){
    char* token;

    token = strtok(path, "/");
    while(token != NULL){
        strcpy(filename,token);
        token = strtok(NULL, "/");
    }
}

int save_to_file(int sockfd, char* filename){
    FILE *f = fopen(filename, "wb");
    char buf[1024];
    size_t bytes;

    printf("[LOG] Starting downloading file.\n");
    while((bytes = read(sockfd, buf, sizeof(buf)))) {
        if(bytes < 0){
            printf("[ERROR] Couldn't read from socket.\n");
            return -1;
        }
        if(fwrite(buf, bytes, 1, f) < 0){
            printf("[ERROR] Couldn't write to target file.\n");
        }
        
        //printf("[LOG] Written %ld bytes to target file.\n", bytes);
    }

    printf("[LOG] Finished download.\n");
    return 1;
}

int close_connection(int sockfd){
    printf("[LOG] Closing connection #%d.\n", sockfd);
    return close(sockfd);
}