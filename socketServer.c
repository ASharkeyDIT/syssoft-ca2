/* 
This is the client program
This file is responsible for allowing socket connections to the server, user id and resource checking and file transfer 
Author : Aaron Sharkey
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include"func.h"
#include <grp.h>
#include <pwd.h>

int connSize;
int serverSock;
struct sockaddr_in server;

static void *connectionUtil(void *fdp);

pthread_mutex_t lock_x;

int main()
{

    fflush(stdout);
    puts("Server starting");
    //Server runs on port 9000
    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSock == -1)
    {
        puts("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server.sin_port = htons(9000);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {

        puts("Error binding server");
        exit(EXIT_FAILURE);
    }

    puts("Server is bound, open for client connections");

    while (1)
    {
        //Connect and listen for the clients

        connSize = sizeof(struct sockaddr_in);
        listen(serverSock, 3);

        struct sockaddr_in client;
        int cSID;
        cSID = accept(serverSock, (struct sockaddr *)&client, (socklen_t *)&connSize);

        if (cSID < 0)
        {
            puts("Connection failed to establish");
        }
        else
        {
            // On client connection start thread
            pthread_t clientThread;
            int *fdp = malloc(sizeof(*fdp));
            *fdp = cSID;
            int tCheck = pthread_create(&clientThread, NULL, connectionUtil, (void *)fdp);
        }
    }

    return 0;
}

void *connectionUtil(void *cfdp)
{
    char message[100];
    char fname[100];
    char fBuffer[512];
    int msgSize = 0;

    memset(message, 0, 500);
    // Get the client socket id
    int client = *((int *)cfdp);

    printf("Connection established, ID : %d \n", client);

    while (strcmp(message, "KILL") != 0)
    {
        //Receive the file name
        msgSize = recv(client, message, 100, 0);

        if (msgSize == 0)
        {
            printf("Client Disconnected, id : %d \n", client);
            memset(message, 0, 100);
            strcpy(message, "KILL");
        }
        else
        {
            
            strcpy(fname, message);
            puts(fname);
            memset(message, 0, 100);

            //Receive the userid
            msgSize = recv(client, message, 100, 0);

            if (msgSize == 0)
            {
                printf("Client Disconnected, id : %d \n", client);
                memset(message, 0, 100);
                strcpy(message, "KILL");
            }
            else
            {
                
                char uid[6];
                strcpy(uid, message);
                memset(message, 0, 100);

                long uidCasted;
                char *ptr;
                uidCasted = strtol(uid, &ptr, 10);
                int uidFinal = uidCasted;
                printf("Client UID %d \n", uidFinal);

                //Receive the Username
                msgSize = recv(client, message, 100, 0);

                if (msgSize == 0)
                {
                    printf("Client Disconnected, id : %d \n", client);
                    memset(message, 0, 100);
                    strcpy(message, "KILL");
                }
                else
                {
                    char user[30];
                    strcpy(user, message);
                    printf("Client Username : %s \n", user);
                    memset(message, 0, 100);

                    char file_buffer[512];
                    char ftemp[50] = "tmp/tmp";
                    strcat(ftemp, uid);

                    FILE *file_open = fopen(ftemp, "w");
                    if (file_open == NULL)
                        printf("File %s Cannot be opened file on server.\n", fname);
                    else
                    {
                        bzero(file_buffer, 512);
                        int block_size = 0;
                        int i = 0;
                        while ((block_size = recv(client, file_buffer, 512, 0)) > 0)
                        {
                            printf("Data Received %d = %d\n", i, block_size);
                            int write_sz = fwrite(file_buffer, sizeof(char), block_size, file_open);
                            bzero(file_buffer, 512);
                            i++;
                        }
                    }

                    printf("Transfer Completed, attempting owner change\n");
                    fclose(file_open);

                    //Constructs a chown command to change the owner of the temporary file to the connected user
                    char chownCmd[150] = {"chown "};
                    strcat(chownCmd, user);
                    strcat(chownCmd, " ");
                    strcat(chownCmd, ftemp);

                    printf("Chown command issued : %s \n", chownCmd);

                    if (system(chownCmd) == -1)
                    {
                        printf("Chown command failed \n");
                    }

                    printf("Changing user permissions \n");

                    //Acquire mutex lock
                    pthread_mutex_lock(&lock_x);

                    // Get the groups that the user belongs to in a list form
                    int *gids;
                    gid_t groupings[5] = {};
                    gids = getGidsServer(user);

                    int gLoop = 0;
                    int marker = 0;
                    // Loop through all group ids
                    while (gLoop < 30)
                    {

                        switch (gids[gLoop])
                        {
                        // If the user is apart of sales
                        case 1001:
                            groupings[marker] = gids[gLoop];
                            marker ++;
                            break;
                        // If the user is apart of promotions
                        case 1002:
                            groupings[marker] = gids[gLoop];
                            marker ++;
                            break;
                        // If the user is apart of offers
                        case 1003:
                            groupings[marker] = gids[gLoop];
                            marker ++;
                            break;
                        // If the user is apart of offers
                        case 1004:
                            groupings[marker] = gids[gLoop];
                            marker ++;
                            break;
                        // If user is a member of the TUDCORP group who owns the website folder
                        case 1006:
                            groupings[marker] = gids[gLoop];
                            marker++;
                            break;
                        }

                            gLoop++;
                    }

                    printf("UID before %d \n", getuid());

                    // Gather current ID's
                    uid_t uid = getuid();
                    uid_t gid = getgid();
                    uid_t ueid = geteuid();
                    uid_t geid = getegid();

                    //set the id to the current user
                    setgroups(5, groupings);
                    setreuid(uidFinal, uid);
                    setregid(uidFinal, gid);
                    seteuid(uidFinal);
                    setegid(uidFinal);

                    printf("UID is now %d \n", getuid());

                    // Performs a move command on the temp file to rename it and place it in the correct dictionary
                    char cmd[150] = {"mv "};
                    strcat(cmd, ftemp);
                    strcat(cmd, " ");
                    strcat(cmd, fname);

                    printf("Final System Command : %s \n", cmd);

                    if(system(cmd) == -1){
                        printf("System command failed \n");
                    }

                    setreuid(0, uid);
                    setregid(0, gid);
                    seteuid(0);
                    setegid(0);

                    char res[50] = {"File transfer complete :)"};

                    //release mutex
                    pthread_mutex_unlock(&lock_x);

                    // if (send(client, &res, 50, 0) < 0)
                    // {
                    //     puts("Error sending username");
                    //     close(client);
                    // }
                    // else
                    // {

                    // }

                    close(client);
                    pthread_exit(NULL);
                }
            }
        }
    }
}