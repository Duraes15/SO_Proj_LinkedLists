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