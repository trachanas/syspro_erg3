#ifndef DROPBOX_LIST_H
#define DROPBOX_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

#define HASH_SIZE 1011

pthread_mutex_t myMutex;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;

char *list_of_files;
int numOfFiles;
char *message;

int listSize;



typedef struct ListID {
    int portNum;
    char *ip;
    int numOfClients;
    struct ListID *next;
    struct ListID *previous;
} ListID;

typedef struct bufferInfo {
    char *pathname;
    int version;
    char *ip;
    int portNum;
}bufferInfo;


typedef struct buffer_t {

    bufferInfo **info;
    int start;
    int end;
    int count;

}buffer_t;


typedef struct info {
    ListID *listID;
    int bufferSize;
    buffer_t *b;
    char *dir;
}info;





//Functions for list of tuples <IP,portNum>
int search(int ,char *, ListID *);

ListID * append(int ,int , char *, ListID *);

void print(ListID *);

void destroyList(ListID **);

void delete(int ,char *, ListID **);

ListID *getClientsList (char *);

char *getList (ListID *,int ,char *);

void myPerror(char *,int );

char *getMyIP();

//Function for buffer

void initialize (buffer_t *,int );

void place_to_buffer (buffer_t *, int , int, char *, char *, int);

bufferInfo * obtain_from_buffer (buffer_t *, int );

int hash(char  *string);

void listdir(const char *name);

ListID *returnNode (int , char *, ListID *);

void deleteNode(ListID** head_ref, ListID* del);


#endif //DROPBOX_LIST_H


