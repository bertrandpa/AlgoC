/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "serveur.h"

volatile __sig_atomic_t is_running;

/* accepter la nouvelle connection d'un client et lire les données
 * envoyées par le client. En suite, le serveur envoie un message
 * en retour
 */
int recois_envoie_message(int client_socket_fd) {
  char data[1024], reponse[1024], savedata[1024];
  json_msg json_data, json_reponse;
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
  memset(reponse, 0, sizeof(reponse));
  memset(&json_data, 0, sizeof(json_data));
  memset(&json_reponse, 0, sizeof(json_reponse));

  // lecture de données envoyées par un client
  int data_size = read(client_socket_fd, (void *)data, sizeof(data));

  if (data_size < 0) {
    close(client_socket_fd);
    printf("[-]Client %d déconnecté\n", client_socket_fd);
    perror("erreur lecture");
    return (EXIT_FAILURE);
  } else if (data_size == 0) {
    close(client_socket_fd);
    printf("[-]Client %d déconnecté\n", client_socket_fd);
    perror("erreur lecture vide");
    return (EXIT_FAILURE);
  }

  printf("Message recu: \n%s\n", data);
  // TODO parse msg
  strncpy(savedata, data, sizeof(data));
  parse_json(savedata, &json_data);
  strcpy(json_reponse.code, json_data.code);
  // fix
  printf("code = %s\n", json_data.code);

  /*
   * extraire le code des données envoyées par le client.
   * Les données envoyées par le client peuvent commencer par le mot "message :"
   * ou un autre mot.
   *
   */
  // Si le message commence par le mot: 'message:'
  if (strcmp(json_data.code, "message") == 0) {
    //
    renvoie_message(client_socket_fd, &json_data, &json_reponse);
  } else if (strcmp(json_data.code, "nom") == 0) {
    renvoie_nom(client_socket_fd, &json_data, &json_reponse);

  } else if (strcmp(json_data.code, "calcule") == 0) {
    recois_numeros_calcule(client_socket_fd, &json_data, &json_reponse);

  } else if (strcmp(json_data.code, "couleurs") == 0) {
    recois_couleurs(client_socket_fd, &json_data, &json_reponse);

  } else if (strcmp(json_data.code, "balises") == 0) {
    recois_balises(client_socket_fd, &json_data, &json_reponse);
  }

  strcpy(json_reponse.valeurs[1], "END\0");

  to_json(reponse, &json_reponse);
  // reponse[strlen(reponse) - 1] = '\0';
  int nbwrite = write(client_socket_fd, reponse, strlen(reponse));
  if (nbwrite <= 0) {
    close(client_socket_fd);
    printf("[-]Client %d déconnecté\n", client_socket_fd);
    perror("Erreur write");
    return (EXIT_FAILURE);
  }
  printf("[sent] to %d : %s\n", client_socket_fd, reponse);
  // fermer le socket
  close(client_socket_fd);
  printf("[-]Client %d déconnecté\n", client_socket_fd);
  return 0;
}

int save(char *path, json_msg *data) {

  FILE *fd = fopen(path, "ab");
  if (fd == NULL) {
    perror("Erreur: open");
    return (EXIT_FAILURE);
  }
  time_t t = time(NULL);
  char *asct = asctime(localtime(&t));
  fprintf(fd, "#save at %s", asct);
  // TODO voir si formattage nécessaire avant save
  char string_json[1024];
  to_json(string_json, data);
  fprintf(fd, "%s\n", string_json);
  fclose(fd);
  printf("File %s saved\n", path);
  return 0;
}

void plot(json_msg *data, int nbcouleurs) {

  // Extraire le compteur et les couleurs RGB
  FILE *p = popen("gnuplot -persist", "w");
  printf("Plot\n");
  int count = 0;
  int slice = 360 / nbcouleurs;
  fprintf(p, "set xrange [-15:15]\n");
  fprintf(p, "set yrange [-15:15]\n");
  fprintf(p, "set style fill transparent solid 0.9 noborder\n");
  fprintf(p, "set title 'Top %d colors'\n", nbcouleurs);
  fprintf(p, "plot '-' with circles lc rgbcolor variable\n");
  // test limit
  while (count < nbcouleurs) {
    char *couleur = data->valeurs[count + 1];
    // TODO voir gnuplot avec données en sin + cos pour le floating point
    // TODO en vu du JSON, faire parssage avant et itérer sur un char**
    // Le numéro 36, parceque 360° (cercle) / 10 couleurs = 36
    fprintf(p, "0 0 10 %d %d 0x%s\n", (count - 1) * slice, count * slice,
            couleur + 1);

    count++;
  }
  fprintf(p, "e\n");
  printf("Plot: FIN\n");
  pclose(p);
}

/* renvoyer un message (*data) au client (client_socket_fd)
 */
int renvoie_message(int client_socket_fd, json_msg *data, json_msg *reponse) {
  char message[1000];
  memset(message, 0, sizeof(message));
  printf("Votre réponse (max %ld caracteres): ", sizeof(message));
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  message[strlen(message) - 1] = '\0';
  strcpy(reponse->valeurs[0], message);
  return 0;
}

int renvoie_nom(int client_socket_fd, json_msg *data, json_msg *reponse) {
  strcpy(reponse->valeurs[0], data->valeurs[0]);
  return 0;
}

int recois_numeros_calcule(int client_socket_fd, json_msg *data,
                           json_msg *reponse) {

  char code[10], *operateur = data->valeurs[0], erreur[50];

  memset(code, 0, sizeof(code));
  memset(erreur, 0, sizeof(erreur));
  float operande1, operande2, result;
  operande1 = atof(data->valeurs[1]);
  operande2 = atof(data->valeurs[2]);
  result = 0.0;
  printf("opera : %s, op1 %f, ope2 %f\n", operateur, operande1, operande2);
  // TODO remove float trailing zeros,
  // or do it only at printing/loading to file ?

  // Change to handle several operation
  if (operande1 != 0.0 && strcmp(data->valeurs[2], "END") != 0) {
    if (strcmp(operateur, "+") == 0)
      result = operande1 + operande2;
    else if (strcmp(operateur, "-") == 0)
      result = operande1 - operande2;
    else if (strcmp(operateur, "*") == 0)
      result = operande1 * operande2;
    else if (strcmp(operateur, "/") == 0) {
      if (operande2 == 0)
        strcpy(erreur, "erreur division par zéro");
      else
        result = operande1 / operande2;
    } else
      strcpy(erreur, "erreur opération inconnue");
  } else
    strcpy(erreur, "erreur format données");
  if (strlen(erreur) != 0)
    strcpy(reponse->valeurs[0], erreur);
  else
    sprintf(reponse->valeurs[0], "%f", result);
  return 0;
}

int recois_couleurs(int client_socket_fd, json_msg *data, json_msg *reponse) {
  // strrchr retourne la dernière occurence de ':'
  // (pour la première voir strchr())
  int nbcouleurs = atoi(data->valeurs[0]);
  if (nbcouleurs > 0) {
    char file_name[30];
    // TODO voir comment ajouter le nom reçu du client
    sprintf(file_name, "files/%d%s", client_socket_fd, "couleurs");
    printf("%s\n", file_name);
    save(file_name, data);
    plot(data, nbcouleurs);
  } else {
    strcpy(reponse->valeurs[0], "erreur nombre couleurs");
    return (EXIT_FAILURE);
  }
  strcpy(reponse->valeurs[0], "enregistre");
  return 0;
}

int recois_balises(int client_socket_fd, json_msg *data, json_msg *reponse) {
  int nbbalises = atoi(data->valeurs[0]);
  if (nbbalises > 0) {
    char file_name[30];
    sprintf(file_name, "files/%d%s", client_socket_fd, "balises");
    printf("%s\n", file_name);
    save(file_name, data);
  } else {
    strcpy(reponse->valeurs[0], "erreur nombre balises");
    return (EXIT_FAILURE);
  }
  strcpy(reponse->valeurs[0], "enregistre");

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
    // thread ou fork ?
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
