#pragma once

#include <stdio.h>/*bibliothèque d'entrée/sortie standard*/
#include <unistd.h>/*Importation de la bibliothèque standard d'unix*/
#include <stdlib.h>/*bibliothèque standard*/
#include <string.h>/* Bibliothèque pour les chaines de caracteres */
#include <time.h>/* Bibliothèque pour le random (creationTab) */
#include <pthread.h>/* Bibliothèque de gestion des threads */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h> /* pour le isdigit */

int saisieString(char* str_chaine,int int_taille);
