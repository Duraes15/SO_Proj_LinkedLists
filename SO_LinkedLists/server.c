#include "dserver.h"

/* Cria um novo nó (livro) duplicando strings */
Livro createBook(int id, const char *title, const char *author, int year, const char *path) {
    Livro n = malloc(sizeof(struct LivroNode));
    if (!n) 
        return NULL;
    n->id = id;
    n->year = year;
    n->title = strdup(title);
    n->author = strdup(author);
    n->path = strdup(path);
    n->next = NULL;
    return n;
}

/* Insere no fim da lista */
Livro insertBook(Livro head, Livro novo) {
    if (!novo) 
        return head;
    if (!head)
        return novo;

    Livro cur = head;
    while (cur->next)
        cur = cur->next;
    cur->next = novo;
    return head;
}

Livro findBook(Livro head, int id) {
    for (Livro cur = head; cur; cur = cur->next) {
        if (cur->id == id)
            return cur;
    }
    return NULL;
}

Livro removeBook(Livro head, int id, int *removido) {
    Livro cur = head;
    Livro prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev)
                prev->next = cur->next;
            else
                head = cur->next;
            /* libera o nó */
            free(cur->title);
            free(cur->author);
            free(cur->path);
            free(cur);
            *removido = 1;
            return head;
        }
        prev = cur;
        cur = cur->next;
    }
    return head;
}

void printList(Livro head, int fd) {
    char buf[512];
    for (Livro cur = head; cur; cur = cur->next) {
        /* sprintf escreve no buf e devolve o número de bytes (excluindo o '\0') */
        int len = sprintf(buf, "ID=%d | %s — %s (%d) @ %s\n", cur->id, cur->title, cur->author, cur->year, cur->path);
        if (len < 0) {
            perror("sprintf");
            continue;
        }
        ssize_t w = write(fd, buf, (size_t)len);
        if (w < 0) {
            perror("write");
        }
    }
}

int listLen(Livro head)
{
    int i = 0;
    for (; head ; head = head->next)
        i++;
    return i;
}

void freeList(Livro head) {
    Livro cur = head;
    while (cur) {
        Livro next = cur->next;
        free(cur->title);
        free(cur->author);
        free(cur->path);
        free(cur);
        cur = next;
    }
}

char **split(const char *s, char sep) {
    size_t count = 1;
    for (const char *p = s; *p; p++)
        if (*p == sep) count++;

    char **res = malloc((count+1)*sizeof(char*));
    size_t idx = 0;
    const char *start = s;
    for (const char *p = s; ; p++) {
        if (*p == sep || *p == '\0') {
            size_t len = p - start;
            res[idx] = malloc(len+1);
            memcpy(res[idx], start, len);
            res[idx][len] = '\0';
            idx++;
            if (*p == '\0') break;
            start = p + 1;
        }
    }
    res[idx] = NULL;
    return res;
}

/*
int persistencia(Livro head) {
    char temp[256];
    snprintf(temp, sizeof temp, "%s.temp", META_FILENAME);

    int fd = open(temp, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    if (fd < 0)
        return -1;

    for (Livro cur = head; cur; cur = cur->next) {
        // 1) id e year
        if ((write(fd, &cur->id,   sizeof cur->id)   != sizeof cur->id) ||
            (write(fd, &cur->year, sizeof cur->year) != sizeof cur->year))
        {
            close(fd);
            unlink(temp);
            return -1;
        }

        // 2) título
        uint32_t len = strlen(cur->title) + 1;
        if (write(fd, &len, sizeof len) != sizeof len ||
            write(fd, cur->title, len)  != len)
        {
            close(fd);
            unlink(temp);
            return -1;
        }

        // 3) autor
        len = strlen(cur->author) + 1;
        if (write(fd, &len, sizeof len) != sizeof len ||
            write(fd, cur->author, len) != len)
        {
            close(fd);
            unlink(temp);
            return -1;
        }

        // 4) path
        len = strlen(cur->path) + 1;
        if (write(fd, &len, sizeof len) != sizeof len ||
            write(fd, cur->path, len)   != len)
        {
            close(fd);
            unlink(temp);
            return -1;
        }
    }

    // fecha o temporário
    if (close(fd) < 0) 
    {
        unlink(temp);
        return -1;
    }

    // substituição atómica para "Ficheirotemp"
    if (rename(temp, META_FILENAME) < 0) 
    {
        unlink(temp);
        return -1;
    }

    return 0;
}*/

char *getTextFromFile(int fd)
{
    char buffer[512];
    size_t total = 0;
    size_t capacity = sizeof buffer;
    char *str = malloc(capacity + 1);

    if (!str)
        return NULL;

    ssize_t n;
    while ((n = read(fd, buffer, sizeof buffer)) > 0)
    {
        if ((size_t)n < sizeof buffer)
            buffer[n] = '\0';
        else
            buffer[sizeof buffer - 1] = '\0';

        // Verificação de espaço
        if (total + (size_t)n + 1 > capacity)
        {
            // ajuste no espaço em 2x
            size_t newcap = capacity * 2;
            while (newcap < total + (size_t)n + 1)
                newcap *= 2;
            char *tmp = realloc(str, newcap + 1);
            if (!tmp)
            {
                free(str);
                return NULL;
            }
            str = tmp;
            capacity = newcap;
        }

        // Copiar os bytes lidos
        memcpy(str + total, buffer, n);
        total += (size_t)n;
    }

    if (n < 0)
    {
        // erro no read
        free(str);
        return NULL;
    }
    str[total] = '\0';
    return str;
}

int numeroLinhas(const char *fifo, Livro indices, int id, const char *keyword)
{
    Livro designado = NULL;
    for (Livro cur = indices; cur; cur = cur->next) {
        if (cur->id == id) {
            designado = cur;
            break;
        }
    }

    int fdFIFO = open(fifo, O_WRONLY | O_NONBLOCK, 0666);
    if (fdFIFO < 0) {
        perror("open fifo");
        return -1;
    }

    if (!designado) {
        write(fdFIFO, "ID inválido\n", 13);
        close(fdFIFO);
        return 0;
    }

    int fdLivro = open(designado->path, O_RDONLY);
    if (fdLivro < 0) 
    {
        perror("open livro");
        close(fdFIFO);
        return -1;
    }

    char *texto = getTextFromFile(fdLivro);
    close(fdLivro);
    if (!texto) 
    {
        const char *msg = "Numero de linhas com essa keyword: 0\n";
        write(fdFIFO, msg, strlen(msg));
        close(fdFIFO);
        return 1;
    }

    int nLinhas = 0;
    char **separada = split(texto, '\n');
    for (int i = 0; separada[i]; i++)
    {
        char *linhaComKeyword = strstr(separada[i], keyword);
        if (linhaComKeyword)
            nLinhas++;
        free(separada[i]);
    }

    free(separada);
    free(texto);

    char strRetorno[64];
    int len = snprintf(strRetorno, sizeof(strRetorno), "Numero de linhas com essa keyword: %d\n", nLinhas);
    if (len < 0) {
        close(fdFIFO);
        return -1;
    }
    write(fdFIFO, strRetorno, (size_t)len);
    close(fdFIFO);
    return 1;
}

int procuraID(char *fifo, int id, Livro indices)
{
    Livro procurado = NULL;
    for (; indices; indices = indices->next)
    {
        if (indices->id == id)
        {
            procurado = indices;
            break ;
        }
    }

    int fdFIFO = open(fifo, O_WRONLY, 0666);
    if (fdFIFO < 0) {
        perror("open fifo");
        return -1;
    }
    // Caso seja um indice invalido
    if (!procurado)
    {
        write(fdFIFO, "ID invalido\n", 13);
        close(fdFIFO);
        return 0;
    }
    char str[512];
    int bytes_lidos = sprintf(str, "Title: %s | Author: %s | Year: %d | Path: %s\n", procurado->title, procurado->author, procurado->year, procurado->path);
    printf("str = %s\n",str);
    write(fdFIFO, str, bytes_lidos);
    close(fdFIFO);
    return 1;
}

int nGivenSigns(char *str, char c)
{
    int counter = 0;
    for (int i = 0; str[i]; i++)
    {
        if (str[i] == c)
            counter++;
    }
    return counter;
}

char **parsing(char *fifoName)
{
    //printf("Entrou no parsing com fifoName = %s\n", fifoName);
    int fdFIFO = open(fifoName, O_RDONLY, 0666);
    //printf("WTF\n");
    if (fdFIFO < 0)
    {
        perror("FIFO not opened");
        return NULL;
    }

    char str[512];
    int bytesRead = read(fdFIFO, &str, 511);
    if (bytesRead < 0)
    {
        perror("ERRO");
        close(fdFIFO);
        return NULL;
    }
    str[bytesRead] = '\0';
    //printf("str = %s\n", str);

    int nArgs = nGivenSigns(str, '|');
    char **strs = malloc(sizeof(char *) * (nArgs + 2));
    if (!strs)
    {
        perror("ERRO");
        close(fdFIFO);
        return NULL;
    }
    //printf("Tamanho dos strs = %ld\n", sizeof(strs));
    for (int i = 0; i < (nArgs + 1); i++)
    {
        strs[i] = malloc(sizeof(char) * 512);
    }
    strs[nArgs + 1] = NULL;

    int i = 0; // indice de str
    int j = 0; // indice das strings do char** args[k][j]
    int k = 0; // indica a string que estamos a escrever (args[k])

    while (str[i] != '\0')
    {
        //printf("Caractere atual = %c\n", str[i]);
        if (str[i] == '|')
        {
            strs[k][j] = '\0';
            j = 0;
            k++;
        }
        else
        {
            strs[k][j] = str[i];
            j++;
        }
        i++;
    }
    strs[k][j] = '\0';

    close(fdFIFO);
    return (strs);
}

int indexaDoc(Livro *indices, char *title, char *authors, int year, char *path, char *fifo, int *ID)
{
    Livro novoLivro = createBook(*ID, title, authors, year, path);
    if (!novoLivro) {
        perror("Falha ao criar livro\n");
        return -1;
    }
    if (novoLivro){
        (*(ID))++;
        Livro novaLista = insertBook(*indices, novoLivro);
        *indices = novaLista;
    } else {
        perror("Creation of newBook Error");
    }

    int fd = open(fifo, O_WRONLY, 0666);
    if (fd < 0) {
        perror("open fifo");
        return -1;
    }

    char entrega[64];
    int bytes = snprintf(entrega, sizeof(entrega), "Livro indexado com id: %d\n", novoLivro->id);
    write(fd, entrega, bytes);
    close(fd);
    return 1;
}

int removeDoc(Livro *indices, int id, char *fifo)
{
    int removido = 0;
    Livro newList = removeBook(*indices, id, &removido);
    *indices = newList;
    int fd = open(fifo, O_WRONLY, 0666);
    char buffer[64];
    if (removido) {
        sprintf(buffer, "Livro com id %d removido com sucesso\n", id);
        write(fd, buffer, strlen(buffer));
        close(fd);
        return 1;
    } else {
        sprintf(buffer, "Livro com id %d nao encontrado\n", id);
        write(fd, buffer, strlen(buffer));
        close(fd);
        return 0;
    }
}

int listaIdDocs(char *keyword, Livro indices, char *fifo)
{
    Livro listaComKeywords = NULL;

    for (Livro cur = indices; cur; cur = cur->next) {
        int fdLivro = open(cur->path, O_RDONLY);
        if (fdLivro < 0) {
            perror("open livro");
            continue;
        }

        char *texto = getTextFromFile(fdLivro);
        close(fdLivro);
        if (texto) {
            if (strstr(texto, keyword)) {
                Livro temp = createBook(cur->id, cur->title, cur->author, cur->year, cur->path);
                listaComKeywords = insertBook(listaComKeywords, temp);
            }
            free(texto);
        }
    }

    int fd = open(fifo, O_WRONLY | O_NONBLOCK, 0666);
    if (fd < 0) {
        perror("open fifo");
        freeList(listaComKeywords);
        return -1;
    }

    printList(listaComKeywords, fd);
    close(fd);
    freeList(listaComKeywords);
    return 1;
}