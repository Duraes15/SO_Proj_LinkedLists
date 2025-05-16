#include "dserver.h"

int main(int argc, char* argv[]){
    if (argc < 3){
        printf("Poucos Argumentos");
        exit(EXIT_FAILURE);
    }

    char docFolder = argv[1];
    int cache_size = atoi(argv[2]);

    if (cache_size <= 0){
        printf("Erro: cache deve ser positivo!\n");
        exit(EXIT_FAILURE);
    }
    
    int ID = 1;


    if (mkfifo(MAIN_FIFO, 0666) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    char inBuff[512];
    int counter = 1;
    Livro indices = NULL;

    while (1)
    {
        printf("ID = %d\n", ID);
        int fd = open(MAIN_FIFO, O_RDONLY, 0666);
        if (fd == -1)
        {
            perror("open");
            continue;
        }

        ssize_t n = read(fd, inBuff, sizeof(inBuff));
        if (n > 0 && strncmp(inBuff,"Pedido Server",13) == 0) 
        {
            printf("Recebemos pedido de criação de fifo\n");
            char fifoName[32];
            int bytes = sprintf(fifoName, "client_response%d", counter);
            mkfifo(fifoName, 0666);
            printf("Fifo criado:%s\n",fifoName);
            counter++;

            fd = open(MAIN_FIFO,O_WRONLY, 0666);
            if (fd == -1){
                perror("Error opening main_fifo!");
            }
            write(fd,fifoName,bytes);

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return 0;
            }
            pid_t pid = fork();
            if (pid == 0) 
            { // Filho
                close(fd);
                char **strs = parsing(&(fifoName[0]));
                int codeSaida = choose_option(&(fifoName[0]),strs,&indices, &ID,docFolder);
                
                /*if (codeSaida == 3)
                {
                    close(main_fifo);
                    unlink(main_fifo);
                    break;
                }*/

                for (int i = 0; strs[i]; i++)
                    free(strs[i]);
                free(strs);
                close(pipefd[0]);
                serialization(pid, &indices, &ID, pipefd);
                _exit(codeSaida); // escolhe a opçao e manda executar
            }
            else{
                int status;
                close(pipefd[1]);
                serialization(pid, &indices, &ID, pipefd);
                waitpid(pid,&status,0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 3)
                {
                    int fd2 = open(fifoName, O_WRONLY, 0666);
                    write(fd2, "Server Shutdown!", 17);
                    close(fd2);
                    printf("Shutdown requested. Exiting server loop.\n");
                    break;
                }
            } 
        }
        close(fd);
    }
    freeList(indices);
    /*for (int i = 1; i <= counter; i++)
    {
        wait(NULL);
        // os fifos já foram fechados no choose_options
    }*/

    /*if (persistencia(indices) < 0)
    {
        perror("Erro ao gravar persistência\n");
    }
    else
    {
        printf("Persistência gravada em %s\n", META_FILENAME);
    }*/

    close(MAIN_FIFO);
    unlink(MAIN_FIFO);
    return 0;
}



int choose_option(char *fifo, char** s, Livro *indices, int *ID, char *docFolder) {
    //printf("Option:%s\n",s[0]);
    int exitCode;
    if(strlen(s[0]) == 2 && s[0][0] == '-'){
        switch (s[0][1])
        {
            case 'a':
                exitCode = indexaDoc(indices, s[1], s[2], atoi(s[3]), s[4], fifo, ID);
                break;
            
            case 'c':
                exitCode = procuraID(fifo, atoi(s[1]), *indices);
                break;
            
            case 'd':
                exitCode = removeDoc(indices, atoi(s[1]), fifo);
                break;
            
            case 'l':
                exitCode = numeroLinhas(fifo, indices, atoi(s[1]), s[2], docFolder);
                break;
            
            case 's':
                exitCode = listaIdDocs(s[1], *indices, fifo);
                break;
            
            case 'f':
                exitCode = 3;
                return (exitCode);
                break;
            default:
                exitCode = 0;
                int fd = open(fifo, O_WRONLY, 0666);
                char *temp = "Opçao Invalida\n";
                write(fd, temp, strlen(temp));
                close(fd);
                break;
        }
    }

    unlink(fifo);
    return exitCode;
}

void serialization(int pid, Livro *indices, int *idAtual, int pipefd[2])
{
    // aqui o pipe ja entra aberto
    // o numero de nodos inclui ate o nodo [nodeInfo] -> NULL
    if (pid == 0)
    {


        //1º escrever numero de nodos (Done)
        int lenLista = listLen(*indices);
        write(pipefd[1], &lenLista, sizeof(int));

        //2º escrever os nodos
        Livro temp = *indices;
        for (; temp; temp = temp->next)
        {
            int titleLen = strlen(temp->title) + 1;
            write(pipefd[1], &titleLen, sizeof(titleLen));  
            write(pipefd[1], temp->title, titleLen);

            int authorLen = strlen(temp->author) + 1;
            write(pipefd[1], &authorLen, sizeof(authorLen));
            write(pipefd[1], temp->author, authorLen);

            write(pipefd[1], &(temp->year), sizeof(int));

            int pathLen = strlen(temp->path) + 1;
            write(pipefd[1], &pathLen, sizeof(pathLen));
            write(pipefd[1], temp->path, pathLen);

            write(pipefd[1], &(temp->id), sizeof(int));
            /*char *title;
            char *author;
            int year;
            char *path;
            int id;
            struct LivroNode *next; //este aqui não é escrito porque o endereço muda*/
        }
        close(pipefd[1]);
    }
    else
    {

        //1º ler o numero de nodos
        int nNodes;
        read(pipefd[0], &nNodes, sizeof(int));

        //2º ler os nodos
        Livro head = NULL;
        Livro prev = NULL;
        for (int i = 0; i < nNodes; i++)
        {
            Livro nodeAtual = malloc(sizeof(struct LivroNode));
            
            int titleLen;
            read(pipefd[0], &titleLen, sizeof(int));
            nodeAtual->title = malloc(sizeof(char) * titleLen);
            read(pipefd[0], (nodeAtual->title), titleLen * sizeof(char));

            int authorLen;
            read(pipefd[0], &authorLen, sizeof(int));
            nodeAtual->author = malloc(sizeof(char) * authorLen);
            read(pipefd[0], (nodeAtual->author), authorLen * sizeof(char));

            read(pipefd[0], &(nodeAtual->year), sizeof(int));

            int pathLen;
            read(pipefd[0], &pathLen, sizeof(int));
            nodeAtual->path = malloc(sizeof(char) * pathLen);
            read(pipefd[0], (nodeAtual->path), pathLen * sizeof(char));

            read(pipefd[0], &(nodeAtual->id), sizeof(int));

            nodeAtual->next = NULL;

            if (i == 0)
                head = nodeAtual;
            else
                prev->next = nodeAtual;

            prev = nodeAtual;
        }

        //3º atualizar o ID com base no numero de nodos
        *idAtual = nNodes + 1;

        //4º limpar a lista anterior e guardar a nova
        freeList(*indices);
        *indices = head;

        //5º fechar o pipe por completo
        close(pipefd[0]);
    }
}