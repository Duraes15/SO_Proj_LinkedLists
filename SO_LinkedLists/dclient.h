#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define MAIN_FIFO "server_pipe"

typedef struct Pedido {
    pid_t pid;
    int len;
    char mensagem[512];
}   req;

char* build_message(int argc, char *argv[]);