#include "dserver.h"

char *ints_to_string(const int *arr, size_t count) 
{
    if (count == 0) 
    {
        char *empty = malloc(1);
        if (empty)
        {
             empty[0] = '\0';
        }
        return empty;
    }
    size_t max_digits = 11;
    size_t bufsize = count * max_digits
                   + (count - 1)  
                   + 1;           
    char *buf = malloc(bufsize);
    if (!buf)
    { 
        return NULL;
    }
    char *p = buf;
    size_t remaining = bufsize;
    for (size_t i = 0; i < count; i++) 
    {

        int written = snprintf(p, remaining, "%d", arr[i]);
        if (written < 0 || (size_t)written >= remaining) 
        {
            free(buf);
            return NULL;
        }
        p += written;
        remaining -= written;
        
        if (i + 1 < count) 
        {
            if (remaining < 2)
            { 
                free(buf); return NULL; 
            }
            *p++ = ',';
            remaining--;
        }
    }
    *p = '\0';
    return buf;
}

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
    free(s);
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

char *getTextFromFile(int fd) {
    char buffer[512];
    size_t total = 0, capacity = 512;
    char *str = malloc(capacity + 1);
    if (!str) return NULL;

    ssize_t n;
    while ((n = read(fd, buffer, sizeof buffer)) > 0) {
        if (total + (size_t)n + 1 > capacity) {
            size_t newcap = capacity * 2;
            while (newcap < total + (size_t)n + 1)
                newcap *= 2;
            char *tmp = realloc(str, newcap + 1);
            if (!tmp) { free(str); return NULL; }
            str = tmp;
            capacity = newcap;
        }
        memcpy(str + total, buffer, (size_t)n);
        total += (size_t)n;
    }
    if (n < 0) { free(str); return NULL; }

    str[total] = '\0';
    return str;
}


int numeroLinhas(const char *fifo, Livro indices, int id, const char *keyword, char *docFolder)
{
    Livro designado = NULL;
    for (Livro cur = indices; cur; cur = cur->next) {
        if (cur->id == id) {
            designado = cur;
            break;
        }
    }

    int fdFIFO = open(fifo, O_WRONLY, 0666);
    if (fdFIFO < 0) {
        perror("open fifo");
        return -1;
    }

    if (!designado) {
        write(fdFIFO, "ID inválido\n", 14);
        close(fdFIFO);
        return 0;
    }

    char newPath[512];
    snprintf(newPath, sizeof(newPath), "%s/%s", docFolder, designado->path);
    int fdLivro = open(newPath, O_RDONLY, 0666);

    if (fdLivro < 0) {
        perror("Open livro");
        close(fdFIFO);
        return -1;
    }

    char *texto = getTextFromFile(fdLivro);
    close(fdLivro);
    if (!texto) {
        const char *msg = "Numero de linhas com essa keyword: 0\n";
        write(fdFIFO, msg, strlen(msg));
        close(fdFIFO);
        return 1;
    }

    int nLinhas = 0;
    char **separada = split(texto, '\n');
    if (separada) 
    {
        for (int i = 0; separada[i]; i++) 
        {
            if (strstr(separada[i], keyword)) 
            {
                nLinhas++;
            }
            free(separada[i]);
        }
        free(separada);
    }

    char strRetorno[64];
    int len = sprintf(strRetorno, "Numero de linhas com essa keyword: %d\n", nLinhas);

    if (len < 0) {
        close(fdFIFO);
        perror("Erro");
        return -1;
    }

    write(fdFIFO, strRetorno, sizeof(char) * len);
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

char **parsing(char *mensagem)
{
    int nArgs = nGivenSigns(mensagem, '|');
    char **strs = malloc(sizeof(char *) * (nArgs + 2));
    if (!strs)
    {
        perror("ERRO");
        return NULL;
    }
    for (int i = 0; i < (nArgs + 1); i++)
    {
        strs[i] = malloc(sizeof(char) * 512);
    }
    strs[nArgs + 1] = NULL;

    int i = 0; // indice de str
    int j = 0; // indice das strings do char** args[k][j]
    int k = 0; // indica a string que estamos a escrever (args[k])

    while (mensagem[i] != '\0')
    {
        if (mensagem[i] == '|')
        {
            strs[k][j] = '\0';
            j = 0;
            k++;
        }
        else
        {
            strs[k][j] = mensagem[i];
            j++;
        }
        i++;
    }
    strs[k][j] = '\0';
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
    int bytes = snprintf(entrega, sizeof(entrega), "Document %d indexed\n", novoLivro->id);
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

int listaIdDocs(char *keyword, Livro indices, char *fifo, char *docFolder)
{
    int *listaIDs = NULL;
    int len = 0;

    for (Livro cur = indices; cur; cur = cur->next) {
        char newPath[64];
        sprintf(newPath, "%s%s", docFolder, cur->path);
        int fdLivro = open(newPath, O_RDONLY,0666);
        if (fdLivro < 0) {
            perror("open livro");
            continue;
        }

        char *texto = getTextFromFile(fdLivro);
        close(fdLivro);
        if (texto) {
            if (strstr(texto, keyword)) {
                if (len == 0)
                {
                    listaIDs = malloc(sizeof(int) * (++len));
                    listaIDs[0] = cur->id;
                }
                else
                {
                    int *tmp = realloc(listaIDs, sizeof(int) * (++len));
                    listaIDs = tmp;
                    listaIDs[len - 1] = cur->id;
                }
            }
            free(texto);
        }
    }

    int fd = open(fifo, O_WRONLY, 0666);
    if (fd < 0) {
        perror("open fifo");
        free(listaIDs);
        return -1;
    }

    if (len == 0)
    {
        write(fd, "Nenhum livro com essa keyword.\n", 32);
        close(fd);
        return 1;
    }
    
    int size = len;
    char *s = ints_to_string(listaIDs, len);
    write(fd, s, strlen(s));
    free(s);
    free(listaIDs);
    close(fd);
    return 1;
}