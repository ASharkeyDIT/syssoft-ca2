/* 
This is the client program
This file is responsible for allowing socket connections to the server, user id and resource checking and file transfer 
Author : Aaron Sharkey
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>
#include<pthread.h> 

int connSize;
int serverSock;
struct sockaddr_in server;

static void *connectionUtil(void *fdp);

int main(){
    
    fflush(stdout);
    puts("Server starting");
    //Server runs on port 9000
    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSock == -1){ 
        puts("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server.sin_port = htons( 9000 );
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSock,(struct sockaddr *) &server, sizeof(server)) <0){
    
        puts("Error binding server");
        exit(EXIT_FAILURE);
    }

    puts("Server is bound, open for client connections");

    while(1){

        connSize = sizeof(struct sockaddr_in);
        listen(serverSock,3);

        struct sockaddr_in client;
        int cSID;
        cSID = accept(serverSock, (struct sockaddr *)&client, (socklen_t*)&connSize);
        
        if (cSID < 0){
            puts("Connection failed to establish");
        }
        else{

            pthread_t clientThread;
            int *fdp = malloc(sizeof(*fdp));
            *fdp = cSID;
            int tCheck = pthread_create(&clientThread, NULL, connectionUtil,(void*) fdp);
        }
    }

    return 0;
}

void *connectionUtil(void *cfdp){
    char message[100];
    char fname[100];
    char fBuffer[512];
    int msgSize = 0;

    memset(message,0,500);

    int client = *((int*)cfdp);

    printf("Connection established, ID : %d \n", client);

    while(strcmp(message,"KILL") != 0){

        msgSize = recv(client, message, 100, 0);

        if (msgSize == 0){
            printf("Client connected, id : %d \n", client);
            memset(message,0,100);
            strcpy(message,"KILL");
        }
        else
        {
            strcpy(fname, message);
            puts(fname);
            char file_buffer[512]; // Receiver buffer
            char* file_name = fname;
        
            FILE *file_open = fopen(file_name, "w");

            if(file_open == NULL)
                printf("File %s Cannot be opened file on server.\n", file_name);
            else {
                bzero(file_buffer, 512); 
                int block_size = 0;
                int i=0;
                while((block_size = recv(client, file_buffer, 512, 0)) > 0) {
                    printf("Data Received %d = %d \n",i,block_size);
                    fwrite(file_buffer, sizeof(char), block_size, file_open);
                    bzero(file_buffer, 512);
                    i++;
                }
                fclose(file_open);
                printf("Transfer Complete!\n");
                char msg[] = "Transfer complete !";
                send(client, msg, (sizeof(msg) + 1), 0);
            }

            
        }
    }

}