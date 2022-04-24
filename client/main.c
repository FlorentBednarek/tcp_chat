#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <sys/msg.h>
#include <unistd.h>

#include "commands.h"
#include "../utils/signals.h"
#include "../utils/client-structures.h"

int msgid;
int tcp_socket;

static void handler(int sig, siginfo_t *info, void *ctx) {
    printf("Signal %s (%d) reçu\n", get_signal_name(sig), sig);
    msgctl(msgid, IPC_RMID, NULL);
    close(tcp_socket);
    exit(-1);
}

static void handle_signals(int signals[], int count) {
    struct sigaction action;
    memset(&action, '\0', sizeof(action));
    action.sa_sigaction = &handler;

    for (int i=0; i<count; i++) {
        sigaction(signals[i], &action, NULL);
    }
}

void *receive_msg()
{
    int len;
    tcpData message;
    while ((len = recv(tcp_socket, &message, sizeof(message), 0)) > 0) {
        message.message[len] = '\0';
        if(strcmp(message.message, LOGOUT_COMMAND) == 0){
            printf("Vous êtes deconnecté\n");
            //TODO : end connexion to pipe
            break;
        }
        switch (message.type) {
            case 1:
                printf("%s : %s\n", message.username, message.message);
                break;
            case 4:
                msgctl(msgid, IPC_RMID, NULL);
                break;
            case 5:
                printf("Vous êtes connecté\n");
                break;
            case 6:
                printf("Vous êtes deconnecté\n");
                break;
            case 7:
                printf("Vous devez vous connecter d'abord!\n");
                break;
            default:
                printf("Message inconnu reçu du serveur: [%ld] %s\n", message.type, message.message);
                break;
        }
    }
    return NULL;
}

void *TCP_connexion(void* args){
    char message[REQUEST_DATA_MAX_LENGTH];
    tcp_socket = socket( AF_INET, SOCK_STREAM,0);
    struct sockaddr_in adr_s;
    int exit_status = 0;
    pthread_t receiver;
    char token[TOKEN_SIZE];
    strcpy(token, "");

    bzero(&adr_s,sizeof(adr_s));
    adr_s.sin_port = htons(TCP_PORT);
    adr_s.sin_family= AF_INET;
    adr_s.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((connect(tcp_socket ,(struct sockaddr *)&adr_s,sizeof(adr_s))) == -1 ) {
        perror("Connection au socket echoué");
        kill(0, SIGINT);
        pthread_exit(NULL);
    }
    pthread_create(&receiver, NULL, receive_msg, NULL);
    printHelp(); // print help menu

    while (exit_status == 0) {
        saisieString(message, REQUEST_DATA_MAX_LENGTH);
        if (commande_detection(message, &exit_status, &(*token), tcp_socket) == 0){ //There is no command
            write(tcp_socket, message, strlen(message));
        }
    }
    close(tcp_socket);
    printf("[Connection-TCP] - Fin de la connection !\n");
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    pthread_t tcp_connect;

    int signals[6] = {SIGSTOP, SIGABRT, SIGINT, SIGQUIT, SIGTERM, SIGTSTP};
    handle_signals(signals, sizeof(signals)/sizeof(signals[0]));
  
    printf("Creation de la connection TCP... ");
    if (pthread_create( &tcp_connect, NULL, TCP_connexion, NULL)) {
        perror("\nErreur durant la creation du thread");
        msgctl(msgid, IPC_RMID, NULL);
    } else
        printf("Created\n");

    pthread_join(tcp_connect, NULL);

    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
