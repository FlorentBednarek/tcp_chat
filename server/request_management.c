#include "request_management.h"

void *login(void* args){
    struct request_processing *parent_info = args;

    int separator_pos;
    char username[MAX_USER_USERNAME_LENGTH], password[MAX_USER_PASSWORD_LENGTH];
    char token[TOKEN_SIZE];

    char data[REQUEST_DATA_MAX_LENGTH];
    strcpy(data,(*parent_info).request.data);

    printf("\tLogin-thread - Received data (length : %ld): %s\n", strlen(data), data); //Log

    for (separator_pos = 0; separator_pos < strlen(data) && data[separator_pos] != USER_PASSWORD_REQUEST_SEPARATOR; separator_pos++);

    if (separator_pos >= MAX_USER_USERNAME_LENGTH || strlen(data)-separator_pos > MAX_USER_PASSWORD_LENGTH){
        (*parent_info).request.type = -1; //There is an error
        strcpy((*parent_info).request.data,"Nom ou mdp trop long");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    } else if (separator_pos == 0 || separator_pos == strlen(data) || separator_pos == strlen(data)-1){
        (*parent_info).request.type = -1;
        strcpy((*parent_info).request.data,"Nom ou mdp vide");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    }

    strncpy(username,data,separator_pos);
    username[separator_pos]='\0';
    strncpy(password,&data[separator_pos]+1,strlen(data)-separator_pos);

    if(findNickname(username,password,ACCOUNT_FILE,1) != 1){
        (*parent_info).request.type = -1; 
        strcpy((*parent_info).request.data,"Wrong username/password");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }else{
        switch (add_user((*parent_info).shared_memory,username,&(*token))){
        case 0:
            (*parent_info).request.type = 0;
            strcpy((*parent_info).request.data,token);
            sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
            break;
        case 1:
            (*parent_info).request.type = -1; 
            strcpy((*parent_info).request.data,"User already connected");
            sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
            break;
        default:
            (*parent_info).request.type = -1; 
            strcpy((*parent_info).request.data,"Maximum number of simultaneous connections reached");
            sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
            break;
        }
    }
    pthread_exit(NULL);
}

void *logout(void* args){
    struct request_processing *parent_info = args;

    char token[TOKEN_SIZE];


    if(strlen((*parent_info).request.data) != (TOKEN_SIZE-1)){
        (*parent_info).request.type = -1; 
        strcpy((*parent_info).request.data,"The token doesn't have the right format");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }
    printf("Logout-thread - Received data (length : %ld): %s\n", strlen(token), token);
    strcpy(token,(*parent_info).request.data);

    switch (remove_user((*parent_info).shared_memory,token)){
    case 0:
        (*parent_info).request.type = 0;
        strcpy((*parent_info).request.data,"User disconnected");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
        break;
    default:
        (*parent_info).request.type = -1; 
        strcpy((*parent_info).request.data,"User not found");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
        break;
    }
    pthread_exit(NULL);
}

void *account_creation(void* args){
    struct request_processing *parent_info = args;
    int separator_pos;
    char username[MAX_USER_USERNAME_LENGTH], password[MAX_USER_PASSWORD_LENGTH];

    char data[REQUEST_DATA_MAX_LENGTH];
    strcpy(data,(*parent_info).request.data);

    printf("Account_creation-thread - Received data (length : %ld): %s\n", strlen(data), data);

    for (separator_pos = 0; separator_pos < strlen(data) && data[separator_pos] != USER_PASSWORD_REQUEST_SEPARATOR; separator_pos++);

    if (separator_pos >= MAX_USER_USERNAME_LENGTH || strlen(data)-separator_pos > MAX_USER_PASSWORD_LENGTH){
        (*parent_info).request.type = -1;
        strcpy((*parent_info).request.data,"Username or password are too long");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    }else if (separator_pos == 0 || separator_pos == strlen(data) || separator_pos == strlen(data)-1){
        (*parent_info).request.type = -1;
        strcpy((*parent_info).request.data,"Username or password are empty");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    }

    strncpy(username,data,separator_pos);
    username[separator_pos]='\0';
    strncpy(password,&data[separator_pos]+1,strlen(data)-separator_pos);

    if(creation(username,password,ACCOUNT_FILE) != 1){
        (*parent_info).request.type = -1; 
        strcpy((*parent_info).request.data,"Nom déjà utilisé");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }else{
        (*parent_info).request.type = 0; 
        strcpy((*parent_info).request.data,"Compte créé");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }

    pthread_exit(NULL);
}

void *account_deletion(void* args){
    struct request_processing *parent_info = args;
    int separator_pos;
    char username[MAX_USER_USERNAME_LENGTH], password[MAX_USER_PASSWORD_LENGTH];
    char data[REQUEST_DATA_MAX_LENGTH];
    strcpy(data,(*parent_info).request.data);

    printf("Account_deletion-thread - Received data (length : %ld): %s\n", strlen(data), data);
    for (separator_pos = 0; separator_pos < strlen(data) && data[separator_pos] != USER_PASSWORD_REQUEST_SEPARATOR; separator_pos++);
    if (separator_pos >= MAX_USER_USERNAME_LENGTH || strlen(data)-separator_pos > MAX_USER_PASSWORD_LENGTH){
        (*parent_info).request.type = -1;
        strcpy((*parent_info).request.data,"nom ou mdp trop long");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    }else if (separator_pos == 0 || separator_pos == strlen(data) || separator_pos == strlen(data)-1){
        (*parent_info).request.type = -1;
        strcpy((*parent_info).request.data,"nom ou mdp vide");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client));
        pthread_exit(NULL);
    }

    strncpy(username,data,separator_pos);
    username[separator_pos]='\0';
    strncpy(password,&data[separator_pos]+1,strlen(data)-separator_pos);
    
    if(findNickname(username,password,ACCOUNT_FILE,1) != 1){
        (*parent_info).request.type = -1; 
        strcpy((*parent_info).request.data,"mauvais nom ou mdp");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }else{
        if(delete(username,ACCOUNT_FILE) != 1){
            (*parent_info).request.type = -1; 
            strcpy((*parent_info).request.data,"Il y a un problème");
            sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
        }else{
            (*parent_info).request.type = 0; 
            strcpy((*parent_info).request.data,"Compte supprimé");
            sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
        }
    }
    
    pthread_exit(NULL);
}

void *connected_users(void* args){
    struct request_processing *parent_info = args;
    
    char connected_list[REQUEST_DATA_MAX_LENGTH];
    strcpy(connected_list,"");
    int bool_empty_list = 1;
    printf("Connected_users-thread - Received data (length : %ld): %s\n", strlen((*parent_info).request.data), (*parent_info).request.data);
    for (size_t i = 0; i < MAX_USERS_CONNECTED; i++)
    {
        if (strcmp((*parent_info).shared_memory[i].username,"") != 0){
            bool_empty_list = 0;
            strcat(connected_list,(*parent_info).shared_memory[i].username);
            connected_list[strlen(connected_list)+1] = '\0';
            connected_list[strlen(connected_list)] = USER_PASSWORD_FILE_SEPARATOR;
        }
    }
    connected_list[strlen(connected_list)-1] = '\0';
    if (bool_empty_list){
        (*parent_info).request.type = 0; 
        strcpy((*parent_info).request.data,"");
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }else{
        (*parent_info).request.type = 0; 
        strcpy((*parent_info).request.data,connected_list);
        sendto ((*parent_info).sock, (void *) &(*parent_info).request, sizeof(struct request), 0, (struct sockaddr *) &(*parent_info).adr_client, sizeof((*parent_info).adr_client)); 
    }

    pthread_exit(NULL);
}