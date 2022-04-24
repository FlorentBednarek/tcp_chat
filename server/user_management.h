#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h> //cwd for get path to users.csv
#include "../utils/request.h"
#include "../utils/utils.h"

// Number of user that can be connected at the same time
#define MAX_USERS_CONNECTED 3
//File of account's username/password
#define ACCOUNT_FILE "users.txt"

struct user{
    char username[MAX_USER_USERNAME_LENGTH]; //Username of the user
    int sock; //Socket descriptor or token if first message not sent
    char token[TOKEN_SIZE];
};

void token_generation(char* res);
int add_user(struct user *shared_memory, char username[MAX_USER_USERNAME_LENGTH], char* token);
int remove_user(struct user *shared_memory, char token[TOKEN_SIZE]);
int numberOfLines(char* path);
char** listOfCouples(char* path);
int findNickname(char* nickname, char* password, char* path, int checkPass);
int creation(char* nickname,char* password,char* path);
int delete(char* nickname,char* path);