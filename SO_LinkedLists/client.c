#include "dclient.h"

char* build_message(int argc, char *argv[]){
    size_t k = 0;
    char *message = malloc(sizeof(char) * 512);
    for (int i = 1; i < argc && i < 6; i++) {
        char *p = argv[i];

        while (*p && k + 1 < 512) 
        {
            message[k++] = *p++;
        }   

        if (i < 5 && k + 1 < 512) 
        {
            message[k++] = '|';
        }
    }

    if (512 > 0) 
    {
        message[k < 512 ? k : 512 - 1] = '\0';
    }
    return message;
}