#pragma once

#include <string.h>
#include "../utils/request.h"
#include "../utils/utils.h"

#define LOGIN_COMMAND ":l"
#define LOGOUT_COMMAND ":o"
#define CREATE_ACCOUNT_COMMAND ":c"
#define DELETE_ACCOUNT_COMMAND ":d"
#define LIST_COMMAND ":u"
#define EXIT_COMMAND ":q"
#define HELP_COMMAND ":h"

void printHelp();
int is_command(char* message, char* command);

void login(char message[REQUEST_DATA_MAX_LENGTH],char* token, struct sockaddr_in adr_s, int udp_socket, int tcp_socket);

void logout(char* token, struct sockaddr_in adr_s, int udp_socket, int tcp_socket, int* exit_status);

void createAccount(char message[REQUEST_DATA_MAX_LENGTH], struct sockaddr_in adr_s, int udp_socket);

void deleteAccount(char message[REQUEST_DATA_MAX_LENGTH], struct sockaddr_in adr_s, int udp_socket);

void connectedUsers(struct sockaddr_in adr_s, int udp_socket);

int commande_detection(char message[REQUEST_DATA_MAX_LENGTH], int* exit_status, char* token, int tcp_sock);