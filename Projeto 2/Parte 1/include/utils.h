#pragma once

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
