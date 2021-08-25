#include "listFd.h"
#include <stdio.h>
#include <stdlib.h>



int searchFd (int fd, fileDes *listFd){

    if (listFd == NULL) return 0;

    fileDes *temp = listFd;

    while (temp != NULL){

        if (temp->fd == fd)
            return 1;

        temp = temp->next;
    }

    return 0;
}


fileDes *appendFd (int fd ,fileDes *listFd){

    if (searchFd(fd,listFd))
        return listFd;

    fileDes *node = malloc(sizeof(fileDes));

    node->fd = fd;

    node->next = NULL;

    if (listFd == NULL){

        listFd = node;
        return listFd;

    }

    fileDes *temp = listFd;

    while (temp->next != NULL){
        temp = temp->next;
    }

    temp->next = node;

    return listFd;

}


void printListFd (fileDes *listFd){

    if (listFd == NULL)
        return;

    fileDes *temp = listFd;

    while (temp != NULL){

        printf("File descriptor is %d\n", temp->fd);

        temp = temp->next;
    }
}

void destroyListFd (fileDes *listFd){

    if (listFd == NULL) return;

    fileDes *node = listFd;

    fileDes *todel;

    while (node != NULL){

        todel = node->next;

        free(node);

        node = todel;
    }

    listFd = NULL;

}

int countFds (fileDes *listFd){

    if (listFd == NULL) return 0;

    int count = 0;

    fileDes *temp = listFd;

    while (temp != NULL){

        count++;

        temp = temp->next;
    }

    return count;

}

//https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/

void deleteNodeFd(fileDes **head_ref, int key)
{
    // Store head node
    struct fileDes* temp = *head_ref, *prev;

    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->fd == key)
    {
        *head_ref = temp->next;   // Changed head
        free(temp);               // free old head
        return;
    }

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->fd != key)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL) return;

    // Unlink the node from linked list
    prev->next = temp->next;

    free(temp);  // Free memory
}














