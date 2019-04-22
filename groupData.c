/* 
This is the groupData program
This file is responsible for gathering the current users group ids
Author : Aaron Sharkey
*/

#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>

int * getGids(){

    // Get current username
    char *user = getenv("USER");
    if(user==NULL) return EXIT_FAILURE;
    
    int ngroups = 50;
    int j;
    gid_t *groups;
    struct passwd *pw;
    struct group *gr;

    // Allocate memory for group id array
    groups = malloc(ngroups * sizeof (gid_t));

    if (groups == NULL){
        printf("Group malloc error \n");
        exit(EXIT_FAILURE);
    }
    //Gather pw structure as it houses some of the gids
    pw = getpwnam(user);

    if (pw == NULL){
        printf("Pw error");
        exit(EXIT_FAILURE);
    }
    // Get the group ids associated with the user and change size of ngroups to reflect the correct value
    getgrouplist(user,pw->pw_gid, groups, &ngroups);

    return groups;
}