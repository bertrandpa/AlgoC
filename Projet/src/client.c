/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "bmp.h"
#include "client.h"

/*
 * Fonction d'envoi et de réception de messages
 * Il faut un argument : l'identifiant de la socket
 */
int envoie_recois_message(int socketfd) {

  char data[1024];
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
  // Demandez à l'utilisateur d'entrer un message
  char message[1012];
  printf("Votre message (max 1000 caracteres): ");
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  fflush(stdout);
  strcpy(data, "message: ");
  strcat(data, message);
  data[strlen(data)] = '\0';

  int write_status = write(socketfd, data, strlen(data));
  if (write_status < 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }

  memset(data, 0, sizeof(data));

  // lire les données de la socket
  int read_status = read(socketfd, data, sizeof(data));
  if (read_status < 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  }

  printf("[recv] %s\n", data);

  return 0;
}

int envoie_nom_de_client(int socketfd) {
  char data[1024];
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));

  // Demandez à l'utilisateur d'entrer un message
  char message[40];
  printf("Votre nom (max 20 caracteres): ");
  fgets(message, sizeof(message) - 1, stdin);
  strcpy(data, "nom: ");
  strcat(data, message);
  data[strlen(data)] = '\0';

  int write_status = write(socketfd, data, strlen(data));
  if (write_status < 0) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  memset(data, 0, sizeof(data));
  int read_status = read(socketfd, data, sizeof(data));
  if (read_status < 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  }

  printf("[recv] %s\n", data);

  return 0;
}

int envoie_operateur_numeros(int socketfd) {
  char data[1024];
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));

  // Demandez à l'utilisateur d'entrer un message
  char message[1000];
  printf("Votre calcul infixe (max 1000 caracteres): ");
  fgets(message, sizeof(message), stdin);
  // TODO analsye data
  /* char operator;
  int operand1, operand2;
  scanf("%c %d %d", &operator, &operand1, &operand2) */
  strcpy(data, "calcule: ");
  strcat(data, message);

  int write_status = write(socketfd, data, strlen(data));
  if (write_status < 0) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  memset(data, 0, sizeof(data));
  int read_status = read(socketfd, data, sizeof(data));
  if (read_status < 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  }

  printf("[recv] %s\n", data);
  return 0;
}

void analyse(char *pathname, char *data) {
  // compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);

  int count;
  strcpy(data, "couleurs: ");
  char temp_string[10] = "10,";
  if (cc->size < 10) {
    sprintf(temp_string, "%d,", cc->size);
  }
  strcat(data, temp_string);

  // choisir 10 couleurs
  for (count = 1; count < 11 && cc->size - count > 0; count++) {
    if (cc->compte_bit == BITS32) {
      sprintf(temp_string, "#%02x%02x%02x,",
              cc->cc.cc24[cc->size - count].c.rouge,
              cc->cc.cc32[cc->size - count].c.vert,
              cc->cc.cc32[cc->size - count].c.bleu);
    }
    if (cc->compte_bit == BITS24) {
      sprintf(temp_string, "#%02x%02x%02x,",
              cc->cc.cc32[cc->size - count].c.rouge,
              cc->cc.cc32[cc->size - count].c.vert,
              cc->cc.cc32[cc->size - count].c.bleu);
    }
    strcat(data, temp_string);
  }

  // enlever le dernier virgule
  data[strlen(data) - 1] = '\0';
}

int envoie_couleurs(int socketfd, char *pathname) {
  char data[1024];
  memset(data, 0, sizeof(data));
  analyse(pathname, data);

  int write_status = write(socketfd, data, strlen(data));
  printf("Couleurs envoyées : %s\n", data);
  if (write_status < 0) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int main(int argc, char **argv) {
  int socketfd;
  int bind_status;

  struct sockaddr_in server_addr, client_addr;

  /*
   * Creation d'une socket
   */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // détails du serveur (adresse et port)
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // demande de connection au serveur
  int connect_status =
      connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (connect_status < 0) {
    perror("connection serveur");
    exit(EXIT_FAILURE);
  }
  // envoie_recois_message(socketfd);
  // envoie_nom_de_client(socketfd);
  envoie_operateur_numeros(socketfd);
  // envoie_couleurs(socketfd, argv[1]);

  close(socketfd);
}
