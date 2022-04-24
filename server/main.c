#include <signal.h>
#include <errno.h>

#include "../utils/utils.h"
#include "../utils/request.h"
#include "../utils/signals.h"
#include "../utils/client-structures.h"
#include "user_management.h"
#include "request_management.h"

extern int errno;
static int IS_RUNNING = true;
int sock_s;
struct user *shared_memory;

static void handler(int sig, siginfo_t *info, void *ctx) {
    printf("Signal %s (%d) reçu\n", get_signal_name(sig), sig);
    IS_RUNNING = false;
    sigaction(sig, &noaction, NULL);
    shutdown(sock_s, SHUT_RD);
    close(sock_s);
    sleep(1);
    munmap(shared_memory, MAX_USERS_CONNECTED*sizeof(char*));
    kill(0, sig);
}

static void handle_signals(int signals[], int count) {
    struct sigaction action;
    memset(&action, '\0', sizeof(action));
    action.sa_sigaction = &handler;

    for (int i=0; i<count; i++) {
        sigaction(signals[i], &action, NULL);
    }
}

void broadcastMessage(char message[REQUEST_DATA_MAX_LENGTH], struct user *shared_memory, int sender_memory_index){
    tcpData broadcast_message;
    broadcast_message.type = 1;
    strcpy(broadcast_message.username, shared_memory[sender_memory_index].username);
    strcpy(broadcast_message.message, message);
    for (size_t i = 0; i < MAX_USERS_CONNECTED; i++)
    {
        if (shared_memory[i].sock != 0) {
            send(shared_memory[i].sock, &broadcast_message, sizeof(broadcast_message),0);
        }
    }
}

void* message_receiver(void* args) {
    struct tcp_informations *arguments = args;
    int sock_c = (*arguments).sock_c;
    char message[REQUEST_DATA_MAX_LENGTH];
    int message_length;
    int memory_index = -1;

    while (IS_RUNNING) {
        message_length = recv(sock_c, message, REQUEST_DATA_MAX_LENGTH, MSG_DONTWAIT);
        if (message_length < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) { // no message, wait and try again
            sleep(1);
            errno = 0;
            continue;
        }
        if (message_length < 1)
            break;
        message[message_length] = '\0';

        if (memory_index == -1) {
            for (size_t i = 0; i < MAX_USERS_CONNECTED; i++)
            {
                if(strcmp(message,(*arguments).shared_memory[i].token) == 0) {
                    (*arguments).shared_memory[i].sock = sock_c;
                    memory_index = i;
                }
            }
            if (memory_index == -1) {
                tcpData msg = {7, "", "Il faut se connecter pour envoyer un message!"};
                send(sock_c, &msg, sizeof(msg), 0);
            } else {
                tcpData msg = {5, "", "Connection réussi!"};
                send(sock_c, &msg, sizeof(msg), 0);
            }
        } else {
            if (strcmp(message, ":o") == 0) {
                tcpData msg = {6, "", ":o"};
                send(sock_c, &msg, sizeof(msg), 0);
                break;
            }
            printf("Message received (%ld): %s\n", strlen(message), message);
            broadcastMessage(message, (*arguments).shared_memory, memory_index);
        }
    }
    printf("[Thread-client] - Fermeture de la connection TCP %d\n", memory_index);
    close(sock_c);
    if (memory_index != -1 && (*arguments).shared_memory[memory_index].sock == sock_c) {
        (*arguments).shared_memory[memory_index].sock = 0;
        strcpy((*arguments).shared_memory[memory_index].token,"");
        strcpy((*arguments).shared_memory[memory_index].username,"");
    }
    pthread_exit(NULL);
}

void *communication(void* args){
    struct user *shared_memory = args;
    struct sockaddr_in adr_s;
    struct tcp_informations thread_infos;
    sock_s = socket( AF_INET , SOCK_STREAM, 0 );
    int sock_c = 0;
    pthread_t client_thread;

    bzero(&adr_s,sizeof(adr_s));
    adr_s.sin_family = AF_INET;
    adr_s.sin_port = htons(TCP_PORT);
    adr_s.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sock_s, (struct sockaddr *)&adr_s, sizeof(adr_s)) == -1){
        perror("[Communication] - Echec liaison serveur");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_s ,REQUEST_DATA_MAX_LENGTH) == -1){
        perror("[Communication] - Echec liaison");
        exit(EXIT_FAILURE);
    }

    while (IS_RUNNING) {
        if ((sock_c = accept(sock_s, (struct sockaddr*) NULL, NULL)) < 0 && IS_RUNNING)
            perror("[Communication] - Echec authorisation");

        thread_infos.shared_memory = shared_memory;
        thread_infos.sock_c = sock_c;

        printf("[Communication] - Creation thread de reception des messages...");
        if (pthread_create(&client_thread, NULL, message_receiver, &thread_infos))
            perror("[Client_thread] - Erreur durant la creation du thread");
        else
            printf("Réussi\n");
    }
    pthread_exit(NULL);
}

void *request_manager(void* args){
    struct user *shared_memory = args;
    unsigned int sock, lg = sizeof(struct sockaddr_in);
    struct sockaddr_in adr_s, adr_c;
    void* thread_function = NULL;
    struct request_processing arguments;

    struct request request;

    pthread_t request_thread;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bzero(&adr_s,sizeof(adr_s)); 
    adr_s.sin_family = AF_INET;
    adr_s.sin_port = htons(UDP_PORT);
    adr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    bind (sock, (struct sockaddr *) &adr_s, sizeof(adr_s)); // Attachement socket

    while (IS_RUNNING) {
        if (recvfrom (sock, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_c, &lg)){

            arguments.adr_client = adr_c;
            arguments.request = request;
            arguments.shared_memory = shared_memory;
            arguments.sock = sock;

            switch (request.type){
                case 1:
                    printf("[Manager de requete] - Creation de thread de connection...\n");
                    thread_function = login;
                    break;
                case -1:
                    printf("[Manager de requete] - Creation de thread de deconnection...\n");
                    thread_function = logout;
                    break;
                case 2:
                    printf("[Manager de requete] - Creation de thread de creation de comptes\n");
                    thread_function = account_creation;
                    break;
                case -2:
                    printf("[Manager de requete] - Creation de thread de suppression de comptes...\n");
                    thread_function = account_deletion;
                    break;
                default:
                    printf("[Manager de requete] - Creation de thread d'affichage des comptes connecté...\n");
                    thread_function = connected_users;
                    break;
            }
            if (pthread_create( &request_thread, NULL, thread_function, &arguments))
                printf("\nErreur durant la creation du request_thread\n");
            else
                pthread_detach(request_thread);
        }
    }
    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    pthread_t com, req;

    srand(time(NULL));

    int signals[6] = {SIGSTOP, SIGABRT, SIGINT, SIGQUIT, SIGTERM, SIGTSTP};
    handle_signals(signals, sizeof(signals)/sizeof(signals[0]));

    shared_memory = mmap(NULL, MAX_USERS_CONNECTED*sizeof(struct user), (PROT_READ | PROT_WRITE), (MAP_SHARED | MAP_ANONYMOUS), -1, 0);
    for (size_t i = 0; i < MAX_USERS_CONNECTED; i++){
        strcpy(shared_memory[i].username,"");
        strcpy(shared_memory[i].token,"");
        shared_memory[i].sock = 0;
    }
    
    printf("Creation du thread de communication.. ");
    if (pthread_create(&com, NULL, communication, (void*)shared_memory)) {
        printf("\nEreeur lors de la creation du thread\n");
        exit(EXIT_FAILURE);
    } else
        printf("Réussi\n");

    printf("Creation du thread de requetes... ");
    if (pthread_create(&req, NULL, request_manager, (void*)shared_memory)) {
        printf("\nEreeur lors de la creation du thread\n");
        exit(EXIT_FAILURE);
    } else
        printf("Réussi\n");
    
    printf("Le serveur est prêt ! ...\n");

    pthread_join(com, NULL);
    pthread_join(req, NULL);

    munmap(shared_memory, MAX_USERS_CONNECTED*sizeof(char*));

    printf("Serveur eteint\n");
    return 0;
}
