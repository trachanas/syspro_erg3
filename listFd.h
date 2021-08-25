#ifndef DROPBOX_LISTFD_H
#define DROPBOX_LISTFD_H


typedef struct fileDes{
    int fd;

    struct fileDes *next;
}fileDes;


fileDes *appendFd(int ,fileDes *);

void destroyListFd (fileDes *);

void printListFd (fileDes *);

int searchFd (int , fileDes *);

int countFds (fileDes *);

int returnFd (fileDes *, int);

void deleteNodeFd(fileDes **, int );


#endif //DROPBOX_LISTFD_H
