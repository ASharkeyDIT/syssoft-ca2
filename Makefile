server : socketServer.o
	gcc -o server socketServer.o -lpthread
	
client : socketClient.o groupData.o
	gcc -o client socketClient.o groupData.o

socketServer.o: socketServer.c
	gcc -c socketServer.c -lpthread

socketClient.o: socketClient.c
	gcc -c socketClient.c

groupData.o: groupData.c func.h
	gcc -c groupData.c

clean : 
	rm server socketServer.o client socketClient.o groupData.o
