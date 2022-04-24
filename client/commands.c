#include "commands.h"

void printHelp() {
    printf("%s = Se connecter au serveur\n", LOGIN_COMMAND);
    printf("%s = Se deconnecter au serveur\n", LOGOUT_COMMAND);
    printf("%s = Créer un compte\n", CREATE_ACCOUNT_COMMAND);
    printf("%s = Supprimer un compte\n", DELETE_ACCOUNT_COMMAND);
    printf("%s = Afficher la liste des compte connecté\n", LIST_COMMAND);
    printf("%s = Quitter le programme\n", EXIT_COMMAND);
}

int is_command(char* message, char* command){
    if((strncmp(message,command,strlen(command)) == 0) 
        && (message[strlen(command)] == ' ' || message[strlen(command)] == '\0')){
        return 1;
    }
    return 0;
}

void login(char message[REQUEST_DATA_MAX_LENGTH],char* token, struct sockaddr_in adr_s, int udp_socket, int tcp_socket){
    struct request request;
    unsigned int lg = sizeof(adr_s);
    /* Building request */
    request.type = 1;
    strcpy(request.data,&message[strlen(LOGIN_COMMAND)+1]);

    /* Log in request (UDP) */
    sendto (udp_socket, (void *) &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, sizeof(adr_s)); 

    /* Receiving token */
    if (recvfrom (udp_socket, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, &lg) > 0) {
        if (request.type == 0) { // Send token to tcp server
            strcpy(token, request.data);
            write(tcp_socket, request.data, strlen(request.data));
        } else { // Wrong username/login
            printf("%s\n", request.data);
        }
    }
}

void logout(char* token, struct sockaddr_in adr_s, int udp_socket, int tcp_socket, int* exit_status){
    struct request request;
    unsigned int lg = sizeof(adr_s);
    /* Building request */
    request.type = -1;
    strcpy(request.data,token);

    /* Send request */
    sendto (udp_socket, (void *) &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, sizeof(adr_s));

    /* Receiving confirmation */
    if (recvfrom (udp_socket, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, &lg) > 0) {
        if (request.type == 0) { //Send deconnection to tcp server
            write(tcp_socket, LOGOUT_COMMAND, strlen(LOGOUT_COMMAND));
            *exit_status = 1;
        } else { // Wrong token
            printf("%s\n", request.data);
        }
    }
}

void createAccount(char message[REQUEST_DATA_MAX_LENGTH], struct sockaddr_in adr_s, int udp_socket){
    struct request request;
    unsigned int lg = sizeof(adr_s);
    /* Building request */
    request.type = 2;
    strcpy(request.data,&message[strlen(CREATE_ACCOUNT_COMMAND)+1]);

    /* Log in request (UDP) */
    sendto (udp_socket, (void *) &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, sizeof(adr_s)); 

    /* Receiving token */
    if (recvfrom (udp_socket, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, &lg)>0){
        if (request.type == 0) {
            printf("%s" "\n",request.data);
        } else { // Something went wrong
            printf("Syntaxe: :c nom mdp\n");
            printf("%s" "\n",request.data);
        }
    }
}

void deleteAccount(char message[REQUEST_DATA_MAX_LENGTH], struct sockaddr_in adr_s, int udp_socket){
    struct request request;
    unsigned int lg = sizeof(adr_s);
    /* Building request */
    request.type = -2;
    strcpy(request.data,&message[strlen(DELETE_ACCOUNT_COMMAND)+1]);

    /* Log in request (UDP) */
    sendto (udp_socket, (void *) &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, sizeof(adr_s)); 

    /* Receiving token */
    if (recvfrom (udp_socket, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, &lg)>0){
        if(request.type == 0){
            printf("%s\n",request.data);
        }else{ //Something went wrong
            printf("Syntaxe: :d nom mdp\n");
            printf("%s\n", request.data);
        }
    }
}

void connectedUsers(struct sockaddr_in adr_s, int udp_socket){
    struct request request;
    unsigned int lg = sizeof(adr_s);
    int counter = 1;

    /* Building request */
    request.type = 0;
    strcpy(request.data,"");

    /* Send request */
    sendto (udp_socket, (void *) &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, sizeof(adr_s));

    /* Receiving list */
    if (recvfrom (udp_socket, &request, sizeof(struct request), 0, (struct sockaddr *) &adr_s, &lg) > 0) {
        if (request.type == 0 && strlen(request.data) > 0) { // Display connected users
            /* Print users username */
            printf("Utilisateur 1: ");
            for (size_t i = 0; i < strlen(request.data); i++)
            {
                if (request.data[i] == '\t') {
                    counter++;
                    printf("\nUtilisateur %d: ", counter);
                } else {
                    printf("%c", request.data[i]);
                }
            }
            printf ("\n");
        } else if (request.type == 0) {
            printf("Personne n'est connecté\n");
        } else { // Something went wrong
            printf("%s\n", request.data);
        }
    }

}

int commande_detection(char message[REQUEST_DATA_MAX_LENGTH], int* exit_status, char* token, int tcp_sock){
    /* ---UDP connection--- */
    struct sockaddr_in adr_s, adr_c;
    unsigned int sock;

    if (strlen(message) > 0 && message[0] == ':') {
        bzero(&adr_c,sizeof(adr_c));
        adr_c.sin_family = AF_INET; 
        adr_c.sin_port = htons(UDP_PORT);
        adr_c.sin_addr.s_addr = htonl(INADDR_ANY);
        //Server init
        bzero(&adr_s,sizeof(adr_s));
        adr_s.sin_family = AF_INET;
        adr_s.sin_port = htons(UDP_PORT);
        adr_s.sin_addr.s_addr = htonl(INADDR_ANY);

        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        bind(sock, (struct sockaddr *) &adr_c, sizeof(adr_c));

        if (is_command(message,LOGIN_COMMAND)) {
            if (strcmp(token,"") != 0) {
                printf("Vous êtes déjà connecté\n");
                return 1;
            }
            if (message[strlen(LOGIN_COMMAND)] == '\0') {
                printf("Syntaxe: :l nom mdp" "\n");
            } else {
                login(message,&(*token), adr_s, sock, tcp_sock);
            }
        } else if (is_command(message,LOGOUT_COMMAND)) {
            logout(token, adr_s, sock, tcp_sock, exit_status);
        } else if (is_command(message,CREATE_ACCOUNT_COMMAND)) {
            createAccount(message,adr_s,sock);
        } else if (is_command(message,DELETE_ACCOUNT_COMMAND)) {
            if (strcmp(token, "") != 0) {
                printf("Il faut être connecté pour supprimer un compte\n");
                return 1;
            }
            deleteAccount(message,adr_s,sock);
        } else if (is_command(message,LIST_COMMAND)) {
            connectedUsers(adr_s,sock);
        } else if (is_command(message,EXIT_COMMAND)) {
            *exit_status = 1;
        } else if (is_command(message,HELP_COMMAND)) {
            printHelp();
        } else {
            printf("Commande inconnu\n");
        }
        return 1;
    }
    return 0;
}