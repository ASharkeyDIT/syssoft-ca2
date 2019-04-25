/* 
This is the client program
This file is responsible for allowing socket connections to the server, user id and resource checking and file transfer 
Author : Aaron Sharkey
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <grp.h>
#include"func.h"

struct sockaddr_in server;

int main(){

    // Get the current users name and greet them
    char *user = getenv("USER");
    if(user==NULL) return EXIT_FAILURE;
    printf("Welcome %s \n", user);

    // Get the groups that the user belongs to in a list form
    int *gids;
    gids = getGids();

    //Get UID and cast it to an integer
    uid_t uid;
    uid = getuid();
    int castedUid = (int) uid;
    char suid[6];
    sprintf(suid,"%d",castedUid);
    printf("UID %s \n", suid);

    // Socket creation and setup
    int connSocket;
    connSocket = socket(AF_INET, SOCK_STREAM, 0);

    if(connSocket == -1){
        puts("Socket creation failed \n");
        exit(EXIT_FAILURE);
    }

    puts("Socket created, attempting server connection \n");
    server.sin_port = htons(9000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;

    int connCheck;
    connCheck = connect(connSocket,(struct sockaddr *)&server, sizeof(server));

    if (connCheck < 0){
        puts("Connection failed \n");
        exit(EXIT_FAILURE);
    }

    // Array that will be used to specify file location for transfer
    char options[5][12] = {
        "",
        "NULL",
        "NULL",
        "NULL",
        "NULL"
    };

    int gLoop = 0;
    int marker = 1;
    printf("Choose a destination for your file \n");
    printf("0 - Root \n");

    // Loop through all group ids
    // If the user is a valid member of a group they will be allowed to select the associated folder at runtime
    while(gLoop < 30){

        switch(gids[gLoop]){
            // If the user is apart of sales
            case 1001:
                printf("%d - Sales \n", marker);
                strcpy(options[marker],"Sales/");
                marker ++;
                break;
            // If the user is apart of promotions
            case 1002:
                printf("%d - Promotions \n", marker);
                strcpy(options[marker],"Promotions/");
                marker ++;
                break;
            // If the user is apart of offers
            case 1003:
                printf("%d - Offers \n", marker);
                strcpy(options[marker],"Offers/");
                marker ++;
                break;
            // If the user is apart of offers
            case 1004:
                printf("%d - Marketing \n", marker);
                strcpy(options[marker],"Marketing/");
                marker ++;
                break;
        }

        gLoop++;
    }

    char fpath[100];
    char fname[100];
    char fRename[100];
    int choice;

    //Gather folder choice
    scanf("%d", &choice);
    fflush(stdin);

    if(choice > marker || choice < 0){
        printf("Invalid choice\n");
        exit(EXIT_FAILURE);
    }
    else{
        printf("option : %s \n",options[choice]);
        strcpy(fname,options[choice]);
    }
    
    // Get file for transfer
    puts("Provide file path for file transfer");
    scanf("%s",fpath);
    fflush(stdin);
    // Provide file a new name
    puts("Provide file name for file transfer");
    scanf("%s",fRename);
    fflush(stdin);
    
    strcat(fname,fRename);
    
    //Open the file for transfer
    FILE *file_open = fopen(fpath, "r");

    if(file_open == NULL){
        puts("Invalid filepath exiting");
        exit(EXIT_FAILURE);
    }
    
    puts("Moving forward with file");
    printf("%s : %s \n", fname, fpath);

    char fBuffer[512];
    char serverResponse[500];
    char clientMessage[500];

 
    //sends the file name so server knows where to write new file to
    if(send(connSocket , fname , (strlen(fname) + 1) , 0) < 0){
            puts("Error sending filename");
    }
    else{
        puts("Sent file name");
        //Sends the user id as a string to the server
        if (send(connSocket, suid, strlen(suid), 0) < 0)
        {
            puts("Error sending uid");
        }
        else{

            puts("Sent uid");
            memset(fname, 0, strlen(fname));
            //Send username to the server
            if (send(connSocket, user, strlen(user), 0) < 0)
            {
                puts("Error sending username");
            }
            else
            {
                puts("Sent username");
                bzero(fBuffer, 512);
                int bSize, blocks = 0;
                // Read from and transfer file to server
                while ((bSize = fread(fBuffer, sizeof(char), 512, file_open)) > 0)
                {
                    printf("File transfer in progress %d = %d \n", blocks, bSize);

                    if (send(connSocket, fBuffer, bSize, 0) < 0)
                    {
                        exit(EXIT_FAILURE);
                    }

                    bzero(fBuffer, 512);
                    blocks++;
                }
                puts("File sending concluded \n");
                fclose(file_open);

                // int msgsSize = 0;
                // char res[50];
                // msgsSize = recv(connSocket, res, 50, 0);

                // if (msgsSize == 0)
                // {
                //     printf("Server issue \n");
                // }
                // else{
                //     printf("Server - %s \n", res);
                // }
            }



        }
    }

    close(connSocket);
}

