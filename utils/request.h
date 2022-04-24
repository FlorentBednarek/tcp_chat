#pragma once


#define UDP_PORT 2058
#define TCP_PORT 2059
#define REQUEST_DATA_MAX_LENGTH 128
#define MAX_USER_USERNAME_LENGTH 10
#define MAX_USER_PASSWORD_LENGTH 10
#define TOKEN_SIZE 16
#define USER_PASSWORD_FILE_SEPARATOR '\t'
#define USER_PASSWORD_REQUEST_SEPARATOR ' '

struct request
{
    int type;
    /** Type correpondence
    * 1 log in
    * -1 log out
    * 2 create account
    * -2 delete account
    * 0 get current connected users
    */
    char data[REQUEST_DATA_MAX_LENGTH];
};
