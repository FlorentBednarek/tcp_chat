#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include <sys/msg.h>
#include <unistd.h>

#include "commands.h"
#include "../utils/utils.h"
#include "../utils/request.h"
#include "../utils/signals.h"
#include "../utils/client-structures.h"

int msgid;
int tcp_socket;

static void handler(int sig, siginfo_t *info, void *ctx) {
    printf("Received signal %s (%d)\n", get_signal_name(sig), sig);
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

/**
*\brief Thread which receive each message from the server and will transfer it to board processus
*
*\param socket TCP socket to listen for new messages
*\return void* Nothing
*/
void *receive_msg()
{
    int len;
    // client thread always ready to receive message
    tcpData message;
    while ((len = recv(tcp_socket, &message, sizeof(message), 0)) > 0) {
        message.message[len] = '\0';
        /* If connection ended */
        if(strcmp(message.message, LOGOUT_COMMAND) == 0){
            printf("You are now logged out\n");
            //TODO : end connexion to pipe
            break;
        }

        /* Send it to message queue */
        switch (message.type) {
            case 1: // messahe se,y
                printf("%s : %s\n", message.username, message.message);
                break;
            case 4: // STOP
                msgctl(msgid, IPC_RMID, NULL);
                break;
            case 5: // connected
                printf("You are now logged in\n");
                break;
            case 6: // disconnected
                printf("You are now disconnected\n");
                break;
            case 7: // you need to log in
                printf("You need to log in first!\n");
                break;
            default:
                printf("Unknwon message from the server: [%ld] %s\n", message.type, message.message);
                break;
        }
    }
    return NULL;
}

/**
*\brief Creation of TCP socket and interception of each connection to affect a thread to connexion
*
*\param args NULL
*\return void* Nothing
 */
void *TCP_connexion(void* args){
    char message[REQUEST_DATA_MAX_LENGTH]; //Message wrote by user
    tcp_socket = socket( AF_INET, SOCK_STREAM,0); //Client socket
    struct sockaddr_in adr_s; //Server address
    int exit_status = 0; //Exit while condition
    pthread_t receiver; //Thread that will receive messages
    char token[TOKEN_SIZE]; //Connexion token
    strcpy(token, "");

    /* Server address init */
    bzero(&adr_s,sizeof(adr_s));
    adr_s.sin_port = htons(TCP_PORT);
    adr_s.sin_family= AF_INET;
    adr_s.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Establish the connection */
    if ((connect(tcp_socket ,(struct sockaddr *)&adr_s,sizeof(adr_s))) == -1 ) {
        perror("Connection to socket failed");
        kill(0, SIGINT);
        pthread_exit(NULL);
    }

    // create a thread to receive messages from server
    pthread_create(&receiver, NULL, receive_msg, NULL);
    printHelp(); // print help menu

    /* Sending messages */
    while (exit_status == 0) {
        saisieString(message, REQUEST_DATA_MAX_LENGTH);
        if (commande_detection(message, &exit_status, &(*token), tcp_socket) == 0){ //There is no command
            write(tcp_socket, message, strlen(message));
        }
    }

    /* Properly end the client */
    close(tcp_socket);
    printf("[TCP-connexion] - Connection ended !\n");
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    pthread_t tcp_connect; // TCP connection

    // add signal handler for potentially-killing signals
    int signals[6] = {SIGSTOP, SIGABRT, SIGINT, SIGQUIT, SIGTERM, SIGTSTP};
    handle_signals(signals, sizeof(signals)/sizeof(signals[0]));

  
    /* Creation of TCP connexion manager */
    printf("Creating TCP thread... ");
    if (pthread_create( &tcp_connect, NULL, TCP_connexion, NULL)) {
        perror("\nError during thread creation");
        msgctl(msgid, IPC_RMID, NULL);
    } else
        printf("Created\n");

    /* Join TCP connexion manager thread */
    pthread_join(tcp_connect, NULL);

    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
