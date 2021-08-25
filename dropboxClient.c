#include "list.h"

char *logOnMessage = "LOG_ON";
char delim[4] = " <>,";

char *clientIP;
int err;
int mySocket;


struct sockaddr_in server;
struct sockaddr *serverPtr = (struct sockaddr *) &server;
struct hostent *hostPtr;
struct in_addr addr;

int portNum;
char *dirName;
int workerThreads;
int bufferSize;
int serverPort;
char *serverIP;
info *infoWorker;
char writeBuffer[1000];



// This is the function for the listener thread. Its created from the dropboxClient in order to accept new connection.
// It is processing "GET_FILE_LIST" and "GET_FILE" request
void * listenThreadFunc (void * args) {

    char b[1000];
    int bytes;
    int listenFd;

    struct sockaddr_in server,client;
    struct sockaddr *serverPtr = (struct sockaddr *)&server;
    struct sockaddr *clientPtr = (struct sockaddr *)&client;
    socklen_t socklen = sizeof(client);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        perror("<Socket creation failed!>");
        return NULL;
    }
 
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(portNum);

    if (bind(sock, serverPtr, sizeof(server)) < 0) {
        perror("<Binding failed!>");
        return NULL;
    }

    if (listen(sock, 3) < 0) {
        perror("<Listening failed!>");
        return NULL;
    }

    while (1) {

        listenFd = accept(sock, clientPtr, &socklen);

        memset(b, '\0', 1000);

        if ((bytes = read(listenFd, b, 1000)) > 0) {

            if (strcmp(b, "GET_FILES_LIST") == 0){

                printf("MESSAGE RECEIVED: %s\n", b);

                list_of_files = calloc(2000, sizeof(char));

                listSize = 0;

                numOfFiles = 0;

                message = calloc(2000, sizeof(char));

                listdir(dirName);

                printf("MESSAGE SENT: %s\n", message);

                write(listenFd, message, strlen(message));

            }
            else if (strcmp(b, "GET_FILE") == 0){



            }

           // free(list_of_files);
           // free(message);
        }

    }
}


void *workerThreadFunc (void * args) {

    info *infoWorker = (info *)args;
    
    int workerSocket;
    struct sockaddr_in workerServer;
    struct sockaddr *workerSerPtr = (struct sockaddr *)&workerServer;
    struct hostent *workerHostPtr;
    struct in_addr workerAddr;
    
    bufferInfo *buffer_info = NULL;

    char c[1000];
    char *token = NULL;
    char *pathname;

    int bytes;
    int version;

    while(1) {

        buffer_info = obtain_from_buffer(infoWorker->b, infoWorker->bufferSize);

        if (buffer_info->pathname == NULL) {

            workerSocket = socket(AF_INET, SOCK_STREAM, 0);
            
            if (workerSocket == -1) {
                perror("<Creating socket failed!>");
                exit(1);
            }

            inet_aton(buffer_info->ip, &workerAddr);

            if ((workerHostPtr = gethostbyaddr(&workerAddr, sizeof(workerAddr), AF_INET)) == NULL) {
                herror("<GetHostByAddr failed!>");
                exit(-1);
            }

            workerServer.sin_family = AF_INET;

            memcpy(&workerServer.sin_addr, workerHostPtr->h_addr, workerHostPtr->h_length);

            workerServer.sin_port = htons(buffer_info->portNum);

            if (connect(workerSocket, workerSerPtr, sizeof(workerServer)) < 0) {
                perror("<Connect 2 failed!>");
                exit(-1);
            }
            
            write(workerSocket, "GET_FILES_LIST", strlen("GET_FILES_LIST"));


            if ((bytes = read(workerSocket,c,1000) > 0)){

                printf("MESSAGE RECEIVED: %s \n", c);

                token = strtok(c, delim);


                //Processing "GET_FILES_LIST" request
                if (strcmp(token, "GET_FILES_LIST") == 0){

                    token = strtok(NULL, delim);
                    token = strtok(NULL, delim);


                    while (token){

                        pathname = strdup(token);
                        token = strtok(NULL, delim);

                        version = atoi(token);
                        token = strtok(NULL, delim);

                        place_to_buffer(infoWorker->b, infoWorker->bufferSize, buffer_info->portNum, buffer_info->ip, pathname, version);

                    }


                }
            }

        }
        else {
            //pathname is not null

            workerSocket = socket(AF_INET, SOCK_STREAM, 0);

            if (workerSocket == -1) {
                perror("<Creating socket failed!>");
                exit(1);
            }

            inet_aton(buffer_info->ip, &workerAddr);

            if ((workerHostPtr = gethostbyaddr(&workerAddr, sizeof(workerAddr), AF_INET)) == NULL) {
                herror("<GetHostByAddr failed!>");
                exit(-1);
            }

            workerServer.sin_family = AF_INET;

            memcpy(&workerServer.sin_addr, workerHostPtr->h_addr, workerHostPtr->h_length);

            workerServer.sin_port = htons(buffer_info->portNum);

            if (connect(workerSocket, workerSerPtr, sizeof(workerServer)) < 0) {
                perror("<Connect failed!>");
                exit(-1);
            }

            write(workerSocket, "GET_FILE", strlen("GET_FILE"));

        }
    }
}

void signal_handler() {


    // Create socket
    mySocket = socket(AF_INET, SOCK_STREAM, 0);

    if (mySocket == -1) {
        perror("<Creating socket failed!>");
        exit(-1);
    }

    // Address from the IP
    inet_aton(serverIP, &addr);

    if ((hostPtr = gethostbyaddr(&addr, sizeof(addr), AF_INET)) == NULL) {
        herror("<GetHostByAddr failed!>\n");
        exit(-1);
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, hostPtr->h_addr, hostPtr->h_length);
    server.sin_port = htons(serverPort);

    if (connect(mySocket, serverPtr, sizeof(server)) < 0) {
        perror("<Connect failed!>");
        exit(-1);
    }

    char *log_off_message = malloc(100 * sizeof(char));

    sprintf(log_off_message, "%s %s %d", "LOG_OFF", clientIP, portNum);

    memset(writeBuffer, '\0', 1000);

    strcpy(writeBuffer,log_off_message);

    printf("MESSAGE SENT: %s\n", log_off_message);

    write(mySocket, writeBuffer, strlen(writeBuffer));

    memset(writeBuffer, '\0', 1000);

    close(mySocket);

    free(log_off_message);

    exit(1);
}

int main(int argc, char *argv[]) {

    ListID *clientList = NULL;

    //Thread Mutex variables
    pthread_t myThread;
    pthread_t listenThread;
    pthread_t *workers;
    pthread_mutex_init(&myMutex, NULL);
    pthread_cond_init(&cond_nonempty, NULL);
    pthread_cond_init(&cond_nonfull, NULL);

    char *pathname = NULL;
    char *message = malloc(100 * sizeof(char));
    char *token = NULL;
    char *rest;
    char *rMessage;
    char *userOnIP;

    int nClientPort = -1;
    int bytes;
    int numOfClients;
    int userOnPort;

    struct in_addr clientAddr;

    if (argc != 13) {
        printf("<Wrong number of arguments!>");
        return -1;
    }

    for (int i = 1; i < argc; i += 2) {

        if (!strcmp(argv[i], "-d")) dirName = strdup(argv[i + 1]);

        else if (!strcmp(argv[i], "-p")) portNum = atoi(argv[i + 1]);

        else if (!strcmp(argv[i], "-w")) workerThreads = atoi(argv[i + 1]);

        else if (!strcmp(argv[i], "-b")) bufferSize = atoi(argv[i + 1]);

        else if (!strcmp(argv[i], "-sp")) serverPort = atoi(argv[i + 1]);

        else if (!strcmp(argv[i], "-sip")) serverIP = strdup(argv[i + 1]);

        else {

            printf("<Wrong arguments!>");
            return -1;
        }
    }

    workers = malloc(workerThreads * sizeof(pthread_t));

    // Create socket
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    
    if (mySocket == -1) {
        perror("<Creating socket failed!>");
        exit(-1);
    }

    // Address from the IP
    inet_aton(serverIP, &addr);

    if ((hostPtr = gethostbyaddr(&addr, sizeof(addr), AF_INET)) == NULL) {
        herror("<GetHostByAddr failed!>\n");
        exit(-1);
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, hostPtr->h_addr, hostPtr->h_length);
    server.sin_port = htons(serverPort);
    
    if (connect(mySocket, serverPtr, sizeof(server)) < 0) {
        perror("<Connect failed!>");
        exit(-1);
    }

    clientIP = getMyIP();

    nClientPort = htons(portNum);

    inet_aton(clientIP, &clientAddr);

    int client_net_address = clientAddr.s_addr;

    //Create "LOG_ON" request
    sprintf(message, "%s %s,%d", logOnMessage, clientIP, portNum);

    printf("MESSAGE SENT: %s\n", message);

    write(mySocket, message, strlen(message));

    //Process "GET_CLIENTS"
    bytes = read(mySocket, writeBuffer, 1000);

    printf("MESSAGE RECEIVED: %s\n", writeBuffer);

    rMessage = strdup(writeBuffer);

    memset(writeBuffer, '\0', 1000);

    clientList = getClientsList(rMessage);

    if (clientList == NULL) numOfClients = 0;

    else numOfClients = clientList->numOfClients;

    buffer_t *buffer = malloc(sizeof(buffer_t));

    initialize(buffer, bufferSize);

    infoWorker = malloc(sizeof(info));

    infoWorker->listID = clientList;
    infoWorker->bufferSize = bufferSize;
    infoWorker->b = buffer;
    infoWorker->dir = strdup(dirName);

    pthread_create(&listenThread, NULL, listenThreadFunc, NULL);

    for (int i = 0; i < workerThreads; i++) pthread_create(&workers[i], NULL, workerThreadFunc, infoWorker);

    signal(SIGINT,signal_handler);

    while (1) {

        read(mySocket, writeBuffer, 1000);

        printf("MESSAGE RECEIVED: %s\n", writeBuffer);

        token = strtok(writeBuffer, delim);

        //Process "USER_ON" messgae
        if (!strcmp(token, "USER_ON")) {

            token = strtok(NULL, delim);

            userOnIP = strdup(token);

            token = strtok(NULL, delim);

            userOnPort = atoi(token);

            numOfClients++;

            place_to_buffer(buffer, bufferSize, userOnPort, userOnIP, NULL, -1);
        }
        else if (!strcmp(token, "USER_OFF")){

            token = strtok(NULL, delim);

            userOnIP = strdup(token);

            token = strtok(NULL, delim);

            userOnPort = atoi(token);

            ListID *temp = returnNode(userOnPort, userOnIP, infoWorker->listID);

            deleteNode(&(infoWorker->listID), temp);

        }

        memset(writeBuffer, '\0', 1000);
    }
}