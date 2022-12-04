#include "utils.h"

int parse_arguments(Arguments *args, char *input){
    // Parse beggining
    if(strncmp(input, "ftp://", 6) != 0){
        //printf("[ERROR] The input given must start with \"ftp://\"\n");
        return -1;
    }

    // Parse user and password if they exist
    if(strchr(input+6, '@') != NULL){
        if(sscanf(input, "ftp://%[^:]:%[^@]@/%[^/]/%s", args->user, args->password, args->host, args->urlPath) != 4){
            //printf("[ERROR] Couldn't parse user and pass\n");
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
            //printf("[ERROR] Couldn't parse host and path\n");
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
        herror("gethostbyname()");
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
        perror("connect()");
        exit(-1);
    }
}

void read_from_socket(int sockfd, char* buffer, size_t size){
    FILE *fp = fdopen(sockfd, "r");
    do {
        buffer = fgets(buffer, size, fp);
        printf("%s", buffer);
    } while (!('1' <= buffer[0] && buffer[0] <= '5') || buffer[3] != ' ');
}