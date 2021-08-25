#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "list.h"
#include "listFd.h"

int current_fd;

int main(int argc, char **argv) {

    //Variables for the arguments and for the execution
    int portNum = -1;
    int mySocket;
    char *token = NULL;
    char *clientIP = NULL;
    int clientPortNum = 0;
    int flag = 0;
    int bytes = -1;
    int numOfClients = 0;
    int fd ;


    //strings for messages process
    char *tuple = NULL;
    char *getClientsMessage = NULL;
    char *message = NULL;
    char readBuffer[1000];
    char writeBuffer[1000];
    char *userOnMessage = malloc(50 * sizeof(char));
    char delim[4] = " ,";

    ListID *listID = NULL;

    //Structs for socket communication
    struct sockaddr_in server,client;
    struct sockaddr *serverPtr = (struct sockaddr *)&server;
    struct sockaddr *clientPtr = (struct sockaddr *)&client;
    struct in_addr clientAddr;
    socklen_t socklen = sizeof(client);


    // Arguments processing
    if (argc != 3)  {
        printf("<Wrong number of arguments!>");
        return -1;
    }

    for (int  i = 1; i < argc; i += 2) {

        if (!strcmp(argv[i], "-p")) portNum = atoi(argv[i+1]);
        
        else {

            printf("<Wrong arguments!>");
            return -1;
        }
    }

    mySocket = socket(AF_INET,SOCK_STREAM,0);

    if (mySocket == -1) {
        perror("<Socket creation failed!>");
        return -1;
    }

    int option = 1;

    // kill "Address already in use" error message || Reuse the same port
    if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portNum);

    //Binding
    if (bind(mySocket,serverPtr, sizeof(server)) < 0) {
        perror("<Socket binding failed!>");
        return -1;
    }

    //Listening
    if (listen(mySocket,3) < 0) {
        perror("<Socket listening failed!>");
        return -1;
    }

    printf("Server is listening in port %d!\n", portNum);

    //List of file descriptors
    fileDes *fdList = NULL;
    fileDes *newList = NULL;

    while (1) {

        // Accept continuously new connections
        fd = accept(mySocket, clientPtr, &socklen);
        
        if (fd < 0) {
            perror("<Accept failed!>");
            exit(-1);
        }

        if ((bytes = read(fd, readBuffer, 1000)) > 0) {

            printf("MESSAGE RECEIVED: %s\n", readBuffer);

            token = strtok(readBuffer, " ,");

            // Processing "LOG_ON" request
            if (!strcmp(token, "LOG_ON")) {

                numOfClients++;

                while (token != NULL) {

                    if (flag == 1) clientIP = strdup(token);
                    
                    else if (flag == 2) clientPortNum = atoi(token);

                    token = strtok(NULL, ",");
                    
                    flag++;
                }

                flag = 0;

                struct in_addr tempAddress;

                tempAddress.s_addr = atoi(clientIP);

                char *temp_addr = inet_ntoa(tempAddress);

                // append the tuple <IP, portNum> to servers list
                listID = append(clientPortNum, numOfClients, clientIP, listID);

                tuple = getList (listID,clientPortNum,clientIP);

                // Create "USER_ON" message
                sprintf(userOnMessage,"%s %s %d", "USER_ON", clientIP, clientPortNum);

                fileDes *temp = fdList;

                // Broadcast "USER_ON" message to other clients
                while (temp != NULL) {

                    memset(writeBuffer, '\0', 1000);

                    strcpy(writeBuffer,userOnMessage);

                    write(temp->fd, writeBuffer, strlen(writeBuffer));

                    temp = temp->next;
                }

                fdList = appendFd(fd, fdList);


                //if clients are more than one connected to dropboxServer
                if (tuple != NULL) {

                    // Creating "CLIENT_LIST" message
                    getClientsMessage = strdup("CLIENT_LIST");

                    message = malloc(strlen(getClientsMessage) + strlen(tuple) + sizeof(int)) ;

                    sprintf(message, "%s %d %s", getClientsMessage, numOfClients - 1, tuple);

                    memset(writeBuffer, '\0', 1000);

                    strcpy(writeBuffer, message);

                    write(fd, writeBuffer, sizeof(writeBuffer));
                    
                    free(message);

                    free(getClientsMessage);
                }

                //If there is only one client in the system
                else {

                    getClientsMessage = strdup("CLIENTS_LIST 0");

                    memset(writeBuffer, '\0', 1000);

                    strcpy(writeBuffer, getClientsMessage);

                    write(fd, writeBuffer, sizeof(writeBuffer));

                    free(getClientsMessage);
                }
            }
            else if (strcmp(token, "LOG_OFF") == 0){


                token = strtok(NULL, delim);

                char *ip_to_del = strdup(token);

                token = strtok(NULL, delim);

                int portNum_to_del = atoi(token);

                if (search(portNum_to_del, ip_to_del, listID)){

                    delete(portNum_to_del, ip_to_del, &listID);

                    fileDes *t = fdList;

                    char *mess = calloc(100, sizeof(char));

                    sprintf(mess, "%s %s %d", "USER_OFF", ip_to_del, portNum_to_del);
                    // Broadcast "USER_ON" message to other clients
                    while (t != NULL) {

                        memset(writeBuffer, '\0', 1000);

                        strcpy(writeBuffer,mess);

                        write(t->fd, writeBuffer, strlen(writeBuffer));

                        t= t->next;
                    }

                    deleteNodeFd(&newList,fd);
                }
                else {
                   printf("ERROR_IP_PORT_NOT_FOUND_IN_LIST\n");
                }

            }

            memset(writeBuffer, '\0', 1000);

            memset(readBuffer, '\0', 1000);
        }
    }
}