#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>

#define META_FILENAME "Ficheirotemp"
#define MAIN_FIFO "server_pipe"

typedef struct LivroNode {
    char *title;
    char *author;
    int year;
    char *path;
    int id;
    struct LivroNode *next;
} *Livro;

int choose_option(char *fifo, char** s, Livro *indices, int *ID);
char **split(const char *s, char sep);
int persistencia(Livro head);
char *getTextFromFile(int fd);
int numeroLinhas(const char *fifo, Livro indices, int id, const char *keyword);
int procuraID(char *fifo, int id, Livro indices);
int nGivenSigns(char *str, char c);
char **parsing(char *fifoName);
int indexaDoc(Livro *indices, char *title, char *authors, int year, char *path, char *fifo, int *ID);
int removeDoc(Livro *indices, int id, char *fifo);
int listaIdDocs(char *keyword, Livro indices, char *fifo);
Livro createBook(int id, const char *title, const char *author, int year, const char *path);
Livro insertBook(Livro head, Livro novo);
Livro findBook(Livro head, int id);
Livro removeBook(Livro head, int id, int *removido);
void printList(Livro head, int fd);
void freeList(Livro head);
int listLen(Livro head);
void serialization(int pid, Livro *indices, int *idAtual, int pipefd[2]);