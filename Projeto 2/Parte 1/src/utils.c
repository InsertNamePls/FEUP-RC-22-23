#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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