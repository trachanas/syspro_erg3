#include "list.h"

//Hashes the input string
int hash(char  *string) {

    unsigned int sum = 0;

    int c;

    while((c = *string++)) sum +=c;

    return sum % HASH_SIZE;
}


//https://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux/29402705
// Lists the files of a folder(subfolders included)
void listdir(const char *name)
{
    DIR *dir;
    struct dirent *entry;

    int s = 0;

    dir = opendir(name);

    int size = 0;

    char *tuple = NULL;

    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {

            char path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)   continue;

            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);

            listdir(path);

        } else if (entry->d_type == DT_REG && strncmp(entry->d_name,".",1) != 0) {

            tuple = calloc(strlen(entry->d_name) + 20, 1);

            numOfFiles++;

            sprintf(tuple, "<%s,%d> ", entry->d_name, hash(entry->d_name));

            list_of_files = realloc(list_of_files, (listSize + strlen(tuple) + 1) * sizeof(char));

            strcat(list_of_files, tuple);

            listSize = (int)strlen(list_of_files);

        }
        //free(tuple);
    }

    closedir(dir);

    sprintf(message, "%s %d %s", "GET_FILES_LIST", numOfFiles, list_of_files);
    //free(list_of_files);

}


void myPerror(char *where,int codeError) {

    fprintf(stderr, "%s: %s\n", where, strerror(codeError));
}

// IP FROM HERE
// https://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
// Gets the IP of the machine
char *getMyIP() {

    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        
        // skip lo and try to find wlsp wifi
        
        if (!ifa->ifa_addr || !strcmp(ifa->ifa_name, "lo")) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            
            char addressBuffer[INET_ADDRSTRLEN];
            
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

            return strdup(addressBuffer);
        }
    }
    printf("IP address not found!\n");
    return NULL;
}


//tokenizes "CLIENT_LIST" and append tuples <IP,portNum> to the list
ListID *getClientsList (char *message) {

    char *token = NULL;

    char *delim = "<> ,";

    ListID *lid = NULL;

    token = strtok(message,delim);

    token = strtok(NULL,delim);

    int numOfClients = atoi(token);

    while (token){

        token = strtok(NULL,delim);

        if (!token) break;

        char *ip = strdup(token);

        token = strtok(NULL,delim);

        int portNum = atoi(token);

        lid = append(portNum,numOfClients,ip,lid);
    }

    return lid;
}


// it returns a string containing all the tuple <IP,portNum> for get_clients request
char * getList(ListID *lid, int portNum, char *ip) {

    if (lid == NULL) return NULL;

    ListID *temp = lid;

    char * tuple = NULL;
    
    int tupleSize = 0;

    while (temp != NULL) {

        if (temp->portNum != portNum || (strcmp(temp->ip,ip) != 0)) {

            char * clientsMes = calloc(strlen(ip) + 20, 1);
            
            sprintf(clientsMes, "<%s,%d> ", temp->ip, temp->portNum);
            
            if (tuple) {
             
                tuple = realloc(tuple, (tupleSize + strlen(clientsMes) + 1) * sizeof(char));
             
                strcat(tuple, clientsMes);
            }

            else {
                
                tuple = calloc(tupleSize + strlen(clientsMes) + 1, sizeof(char));

                strcpy(tuple, clientsMes);
            }

            free(clientsMes);

            tupleSize = strlen(tuple);

        }

        temp = temp->next;
    }

    return tuple;
}

// Searches if a tuple <ip, portnum> exists in the list
int search(int portNum, char *ip, ListID *lid) {
    
    if (lid == NULL) return 0;

    ListID *temp = lid;

    while (temp != NULL) {

        if (temp->portNum == portNum && strcmp(temp->ip, ip) == 0)
        {
            return 1;
        }
        
        temp = temp->next;
    }

    return 0;
}


//Sychronized access to list, in order to append a tuple <IP, portNum>
ListID * append(int portNum, int numOfClients, char * clientIP, ListID *lid) {

    pthread_mutex_lock(&myMutex);

    if (search(portNum, clientIP, lid)) {

        pthread_mutex_unlock(&myMutex);

        return lid;
    }

    ListID *node = malloc(sizeof(ListID));

    node->ip = strdup(clientIP);

    node->portNum = portNum;

    node->next = NULL;

    node->numOfClients = numOfClients;

    if (lid == NULL) {

        node->previous = NULL;

        pthread_mutex_unlock(&myMutex);

        return node;
    }

    ListID *temp = lid;

    while (temp->next != NULL) temp = temp->next;

    temp->next = node;

    node->previous = temp;

    pthread_mutex_unlock(&myMutex);

    return lid;
}

//Prints the list
void print(ListID *lid) {
    
    ListID *temp = lid;

    if (lid == NULL) return;

    while (temp != NULL) {

        printf("IP: %s, Port Number: %d \n\n", temp->ip, temp->portNum);

        temp = temp->next;
    }
}


// free list
void destroyList(ListID **lid) {

    ListID *node = *lid;

    ListID *todel;

    while(node != NULL) {

        todel = node->next;

        free(node);

        node = todel;
    }

    *lid = NULL;
}

//https://www.geeksforgeeks.org/delete-a-node-in-a-doubly-linked-list/
void deleteNode(ListID** head_ref, ListID* del) {

    pthread_mutex_lock(&myMutex);

    /* base case */
    if (*head_ref == NULL || del == NULL) {

        pthread_mutex_unlock(&myMutex);

        return;
    }

    if (*head_ref == del)
        *head_ref = del->next;

    if (del->next != NULL)
        del->next->previous = del->previous;

    if (del->previous != NULL)
        del->previous->next = del->next;

    free(del);

    pthread_mutex_unlock(&myMutex);

    return;
}





// Deletes the given tuple <ip,portnum> from the list
void delete(int portNum, char *ip, ListID **lid) {

    if (!search(portNum, ip, *lid)) return;

    ListID *temp = *lid;

    ListID *prev = temp;

    ListID *todel = returnNode(portNum, ip, *lid);

    deleteNode(lid, todel);

}


ListID *returnNode (int portNum, char *ip, ListID *list){

    ListID *temp = list;

    while (temp != NULL){

        if (temp->portNum == portNum && !strcmp(temp->ip, ip)) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

//Initialize the cyclic buffer
void initialize (buffer_t *buffer,int bufferSize) {

    buffer->info = calloc(bufferSize, sizeof(bufferInfo*));

    buffer->start = 0;

    buffer->end = -1;

    buffer->count = 0;
}


void place_to_buffer (buffer_t *buffer, int bufferSize, int portNum, char *ip, char *pathname,int version) {

    pthread_mutex_lock(&myMutex);

    while (buffer->count >= bufferSize) {

        pthread_cond_wait(&cond_nonfull, &myMutex);
    }

    buffer->end = (buffer->end + 1) % bufferSize;

    buffer->info[buffer->end] = malloc(sizeof(bufferInfo));

    buffer->info[buffer->end]->portNum = portNum;

    buffer->info[buffer->end]->version = (version == -1) ? -1 : version;

    buffer->info[buffer->end]->ip = strdup(ip);

    buffer->info[buffer->end]->pathname = pathname ? strdup(pathname) : NULL;

    buffer->count++;

    pthread_mutex_unlock(&myMutex);

    pthread_cond_signal(&cond_nonempty);
}

bufferInfo* obtain_from_buffer (buffer_t *buffer, int bufferSize) {

    pthread_mutex_lock(&myMutex);
    
    while (buffer->count == 0) pthread_cond_wait(&cond_nonempty, &myMutex);

    bufferInfo *bf = buffer->info[buffer->start];

    buffer->start = (buffer->start + 1) % bufferSize;

    buffer->count--;

    pthread_mutex_unlock(&myMutex);

    pthread_cond_signal(&cond_nonfull);

    return bf;
}