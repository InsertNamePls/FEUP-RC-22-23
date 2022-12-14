#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

// Macros
#define MAX_LENGTH 255

// Structures
typedef struct {
    char user[MAX_LENGTH];
    char password[MAX_LENGTH];
    char host[MAX_LENGTH];
    char urlPath[MAX_LENGTH];
} Arguments;

// Functions
int parse_arguments(Arguments *args, char *input);
int create_socket();
char* getIP(char* hostname);
void connect_socket(int sockfd, char* ip, int port);
void read_from_socket(int sockfd, char* buffer, size_t size);
void send_credentials(int sockfd, char* user, char* password);
void enter_passive_mode(int sockfd);
int get_new_port(char* buffer);
void send_file(int sockfd, char* path);
void getFilename(char* filename, char* path);
int save_to_file(int sockfd, char* filename);
int close_connection(int sockfd);