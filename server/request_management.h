#pragma once

#include "../utils/request.h"
#include "../utils/utils.h"
#include "user_management.h"

struct request_processing{
    struct request request;
    unsigned int sock;
    struct sockaddr_in adr_client;
    struct user *shared_memory;
};

struct tcp_informations{
    int sock_c;
    struct user *shared_memory;
};
void *login(void* args);
void *logout(void* args);
void *account_creation(void* args);
void *account_deletion(void* args);
void *connected_users(void* args);
