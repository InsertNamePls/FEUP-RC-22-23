#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>

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
void read_from_socket(int sockfd, char* msg, size_t size);
