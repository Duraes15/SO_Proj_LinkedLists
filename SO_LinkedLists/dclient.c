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

    char fifoName[32];
    pid_t pid = getpid(); // pid para struct
    req pedidoParaServer;
    sprintf(fifoName, "client_response%d", pid);
    pedidoParaServer.pid = pid;
    char *tmp = build_message(argc, argv);
    int i;
    for (i = 0; tmp[i] != '\0'; i++)
    {
        pedidoParaServer.mensagem[i] = tmp[i];
    }
    pedidoParaServer.mensagem[i] = '\0';
    pedidoParaServer.len = i;
    free(tmp);
    mkfifo(fifoName, 0666);
    
    int fd_mainFIFO = open(MAIN_FIFO, O_WRONLY, 0666);
    if (fd_mainFIFO == -1)
    {
        perror("ERRO");
    }
    
    write(fd_mainFIFO, &(pedidoParaServer.pid), sizeof(int));
    write(fd_mainFIFO, &(pedidoParaServer.len), sizeof(int));
    write(fd_mainFIFO, &(pedidoParaServer.mensagem[0]), sizeof(char) * strlen(pedidoParaServer.mensagem));
    close(fd_mainFIFO);
    // ate aqui a mensagem ja foi enviada atraves do mainFIFO

    char serverResponse[512];
    int fdFIFO = open(fifoName, O_RDONLY, 0666);
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
    unlink(fifoName);
    return 1;
}