all:target

target: dropboxServer.o dropboxClient.o list.o listFd.o
	gcc -o server dropboxServer.o list.o listFd.o -lpthread -g -W
	gcc -o client dropboxClient.o list.o -lpthread -g -W

dropboxServer.o:    dropboxServer.c
	gcc -c dropboxServer.c

dropboxClient.o:    dropboxClient.c
	gcc -c dropboxClient.c

list.o: list.c
	gcc -c list.c

listFd.o: listFd.c
	gcc -c listFd.c

clean:
	rm -rf client server *.o
