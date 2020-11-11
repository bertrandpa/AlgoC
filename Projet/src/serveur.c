/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "serveur.h"

volatile __sig_atomic_t is_running;

/* accepter la nouvelle connection d'un client et lire les données
 * envoyées par le client. En suite, le serveur envoie un message
 * en retour
 */
int recois_envoie_message(int client_socket_fd) {
  struct sockaddr_in client_addr;
  char data[1024];

  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));

  // lecture de données envoyées par un client
  int data_size = read(client_socket_fd, (void *)data, sizeof(data));

  if (data_size < 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  } else if (data_size == 0) {
    perror("erreur lecture vide");
    return (EXIT_FAILURE);
  }

  printf("Message recu: %s\n", data);

  /*
   * extraire le code des données envoyées par le client.
   * Les données envoyées par le client peuvent commencer par le mot "message :"
   * ou un autre mot.
   */

  char code[10];

  sscanf(data, "%s", code);
  data[strlen(data) - 1] = '\0';
  // Si le message commence par le mot: 'message:'
  if (strcmp(code, "message:") == 0) {
    renvoie_message(client_socket_fd, data);
  } else if (strcmp(code, "nom:") == 0) {
    renvoie_nom(client_socket_fd, data);
  } else if (strcmp(code, "calcule:") == 0) {
    recois_numeros_calcule(client_socket_fd, data);
  } else {
    plot(data);
  }

  // fermer le socket
  close(client_socket_fd);
  printf("[-]Client %d déconnecté\n", client_socket_fd);
  return 0;
}

void plot(char *data) {

  // Extraire le compteur et les couleurs RGB
  FILE *p = popen("gnuplot -persist", "w");
  printf("Plot");
  int count = 0;
  int n;
  char *saveptr = NULL;
  char *str = data;
  fprintf(p, "set xrange [-15:15]\n");
  fprintf(p, "set yrange [-15:15]\n");
  fprintf(p, "set style fill transparent solid 0.9 noborder\n");
  fprintf(p, "set title 'Top 10 colors'\n");
  fprintf(p, "plot '-' with circles lc rgbcolor variable\n");
  while (1) {
    char *token = strtok_r(str, ",", &saveptr);
    if (token == NULL) {
      break;
    }
    str = NULL;
    if (count == 0) {
      n = atoi(token);
    } else {
      // Le numéro 36, parceque 360° (cercle) / 10 couleurs = 36
      fprintf(p, "0 0 10 %d %d 0x%s\n", (count - 1) * 36, count * 36,
              token + 1);
    }
    count++;
  }
  fprintf(p, "e\n");
  printf("Plot: FIN\n");
  pclose(p);
}

/* renvoyer un message (*data) au client (client_socket_fd)
 */
int renvoie_message(int client_socket_fd, char *data) {
  char msg[1024];
  memset(msg, 0, sizeof(msg));
  printf("Votre réponse (max %ld caracteres): ", sizeof(msg));
  if (fgets(msg, sizeof(msg), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  fflush(stdout);
  int msg_size = write(client_socket_fd, msg, strlen(msg));
  printf("[send] to %d : %s\n", client_socket_fd, msg);

  if (msg_size < 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }
  return 0;
}

int renvoie_nom(int client_socket_fd, char *data) {
  int data_size = write(client_socket_fd, data, strlen(data));
  printf("[send] to %d : %s\n", client_socket_fd, data);

  if (data_size < 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }
  return 0;
}

int recois_numeros_calcule(int client_socket_fd, char *data) {
  // TODO analsye data
  char message[1024];
  memset(message, 0, sizeof(message));
  char code[10]; // pass en param ?
  memset(code, 0, sizeof(code));
  char operateur[20];
  memset(operateur, 0, sizeof(operateur));
  char erreur[50];
  memset(erreur, 0, sizeof(erreur));
  float operande1, operande2, result;
  operande1 = operande2 = result = 0.0;
  // TODO remove float trailing zeros
  /* sscanf return number of variables filled*/
  int read_value = sscanf((void *)data, "%s %s %f %f", code, operateur,
                          &operande1, &operande2);
  if (read_value > 2) {
    if (strcmp(operateur, "+") == 0)
      result = operande1 + operande2;
    else if (strcmp(operateur, "-") == 0)
      result = operande1 - operande2;
    else if (strcmp(operateur, "*") == 0)
      result = operande1 * operande2;
    else if (strcmp(operateur, "/") == 0) {
      if (operande2 == 0)
        strcpy(erreur, "erreur : division par zéro");
      else
        result = operande1 / operande2;
    } else
      strcpy(erreur, "erreur : opération inconnue");
  } else
    strcpy(erreur, "erreur : format données");
  if (strlen(erreur) != 0)
    sprintf(message, "%s %s", code, erreur);
  else
    sprintf(message, "%s %f", code, result);

  int data_size = write(client_socket_fd, message, strlen(message));
  printf("[send] to %d : %s\n", client_socket_fd, message);

  if (data_size < 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }
  return 0;
}

// TODO fermer toutes les fenêtres gnuplot
void sighandler(int sigint) {
  if (is_running == 0)
    return;
  is_running = 0;
}

int main() {

  int socketfd;
  int bind_status;
  socklen_t client_addr_len;

  struct sockaddr_in server_addr, client_addr;

  struct sigaction action;
  sigemptyset(&action.sa_mask);
  action.sa_handler = sighandler;
  action.sa_flags = 0;
  sigaction(SIGINT, &action, 0);

  /*
   * Creation d'une socket
   */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    perror("Unable to open a socket");
    return -1;
  }

  int option = 1;
  setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  // détails du serveur (adresse et port)
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Relier l'adresse à la socket
  bind_status =
      bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status < 0) {
    perror("bind");
    return (EXIT_FAILURE);
  }

  // Écouter les messages envoyés par le client
  if (listen(socketfd, 10) < 0) {
    perror("listen");
    return (EXIT_FAILURE);
  }
  // TODO fixme
  is_running = 1;
  while (is_running == 1) {
    printf("En attente de client\n");
    int client_socket_fd =
        accept(socketfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket_fd < 0) {
      perror("accept");
      return (EXIT_FAILURE);
    }
    printf("[+]Client %d est connecté\n", client_socket_fd);
    //à thread/fork pour multi client
    /* pid_t pid;
    if ((pid = fork()) < 0) {
      perror("fork");
      return (EXIT_FAILURE);
    } else if (pid == 0) {
      // child
      recois_envoie_message(client_socket_fd);
    } else {
      // parent
      // TODO allouer tab taille fixe nb max child et wait avec lock + predicate
      continue;
    } */
    recois_envoie_message(client_socket_fd);
    // Lire et répondre au client
  }

  close(socketfd);

  return 0;
}
