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
    
    int fd_mainFIFO = open("server_pipe", O_WRONLY);

    if (fd_mainFIFO == -1)
    {
        perror("ERRO");
    }
    write(fd_mainFIFO, "Pedido Server", 14);
    printf("Pedido enviado ao server\n");
    close(fd_mainFIFO);

    fd_mainFIFO = open("server_pipe", O_RDONLY);
    if (fd_mainFIFO == -1)
    {
        perror("ERRO");
    }
    
    char *str = build_message(argc, argv);
    
    char fifoName[32];
    int bytesRead = read(fd_mainFIFO, fifoName, sizeof(fifoName) - 1);
    if (bytesRead <= 0) 
    {
        perror("Read Error");
        close(fd_mainFIFO);
        free(str);
        return 0;
    }
    fifoName[bytesRead] = '\0';

    if (strcmp("Pedido Inválido", fifoName) == 0)
    {
        perror("Erro no Pedido");
    }
    
    close(fd_mainFIFO);

    int fdFIFO = open(fifoName, O_WRONLY, 0666);
    write(fdFIFO, str, strlen(str));
    printf("Enviamos pedido para pipe\n");
    close(fdFIFO);
    free(str);

    char serverResponse[512];
    fdFIFO = open(fifoName, O_RDONLY, 0666);
    read(fdFIFO, &serverResponse, 512);
    printf("Server Response:\n%s\n", serverResponse);
    close(fdFIFO);
}