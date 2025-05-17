#include "dclient.h"

int main(int argc, char* argv[]){

    if ( argc < 2 )
    {
        printf("Erro: argumentos insuficientes\n");
        printf("Uso:\n");
        printf("Adicionar documento: ./dclient -a [titulo] [autores] [ano] [path]\n");
        printf("Consultar Documento: ./dclient -c  [registo]\n");
        printf("Remover Documento: ./dclient -d [registo]\n");
        printf("Contar linhas: ./dclient -l [registo] [nome]\n");
        printf("Listar IDs: ./dclient -s [nome]\n");
        printf("Persistência: ./dclient -f\n");
        return 1;
    }
    
    int fd_mainFIFO = open(MAIN_FIFO, O_WRONLY, 0666);

    if (fd_mainFIFO == -1)
    {
        perror("ERRO");
    }
    pid_t pid = getpid();
    char fifoName[32];
    sprintf(fifoName, "client_response%d", pid);
    mkfifo(fifoName, 0666);
    int fdFIFO = open(fifoName, O_WRONLY, 0666);
    if (fdFIFO == -1)
    {
        perror("ERRO");
    }
    write(fd_mainFIFO, &pid, sizeof(pid_t));
    close(fd_mainFIFO);
    
    char *str = build_message(argc, argv);
    write(fdFIFO, str, strlen(str));
    close(fdFIFO);
    free(str);

    char serverResponse[512];
    fdFIFO = open(fifoName, O_RDONLY, 0666);
    if (fdFIFO < 0){
        perror("Erro ao abrir Fifo de resposta");
    }
    int respBytes = read(fdFIFO, &serverResponse, sizeof(serverResponse) - 1);
    if (respBytes < 0){
        perror("Erro na leitura da resposta");
        close(fdFIFO);
        unlink(fdFIFO);
        return 0;
    }
    serverResponse[respBytes] = '\0';
    printf("Server Response:\n%s\n", serverResponse);
    
    close(fdFIFO);
    unlink(fdFIFO);
    return 1;
}