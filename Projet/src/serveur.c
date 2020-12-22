/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <fcntl.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
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
  json_msg *json_data, *json_reponse;
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
  memset(reponse, 0, sizeof(reponse));
  json_data = malloc(sizeof(json_msg));
  json_reponse = malloc(sizeof(json_msg));
  memset((void *)json_data, 0, sizeof(*json_data));
  memset((void *)json_reponse, 0, sizeof(*json_reponse));

  // lecture de données envoyées par un client
  int data_size = read(client_socket_fd, (void *)data, sizeof(data));

  if (data_size < 0) {
    perror("erreur lecture");
    return EXIT_FAILURE;
  } else if (data_size == 0) {
    perror("erreur lecture vide");
    return EXIT_FAILURE;
  }

  printf("Message recu: \n%s\n", data);
  strncpy(savedata, data, sizeof(data));

  // On parse le message client et on remplit la structure
  if (parse_json(savedata, json_data)) {
    delete_json(json_data);
    delete_json(json_reponse);
    return EXIT_FAILURE;
  }
  strcpy(json_reponse->code, json_data->code);
  printf("code = %s\n", json_data->code);

  /*
   * extraire le code des données envoyées par le client.
   * Les données envoyées par le client peuvent commencer par le mot "message
   * :" ou un autre mot.
   *
   */
  int exit_status;
  // Si le message commence par le mot: 'message:'
  if (strcmp(json_data->code, "message") == 0) {
    exit_status = renvoie_message(json_data, json_reponse);

  } else if (strcmp(json_data->code, "nom") == 0) {
    exit_status = renvoie_nom(json_data, json_reponse);

  } else if (strcmp(json_data->code, "calcule") == 0) {
    exit_status = recois_numeros_calcule(json_data, json_reponse);

  } else if (strcmp(json_data->code, "couleurs") == 0) {
    exit_status = recois_couleurs(json_data, json_reponse);

  } else if (strcmp(json_data->code, "balises") == 0) {
    exit_status = recois_balises(json_data, json_reponse);
  }

  if (exit_status) {
    delete_json(json_data);
    delete_json(json_reponse);
    return EXIT_FAILURE;
  }

  // met la reponse en string au format json
  json_to_string(reponse, json_reponse);

  delete_json(json_data);
  delete_json(json_reponse);

  // renvoie la reponse au client
  int nbwrite = write(client_socket_fd, reponse, strlen(reponse));
  if (nbwrite <= 0) {
    perror("Erreur write");
    return EXIT_FAILURE;
  }
  // Accusé d'envoie
  // printf("[sent] to %d : %s\n", client_socket_fd, reponse);

  return EXIT_SUCCESS;
}

int save(char *path, json_msg *data) {

  FILE *fp = fopen(path, "ab");
  if (fp == NULL) {
    perror("Erreur: open");
    return (EXIT_FAILURE);
  }
  time_t t = time(NULL);
  char *asct = asctime(localtime(&t));
  fprintf(fp, "#save at %s\n", asct);
  fprintf(fp, "%s : ", data->code);
  uint i;
  for (i = 0; i < data->size - 1; i++) {
    fprintf(fp, "%s, ", data->valeurs.str_array[i]);
  }
  fprintf(fp, "%s\n", data->valeurs.str_array[i]);
  fclose(fp);
  printf("File %s saved\n", path);
  return 0;
}

void plot(char **data, int nbcouleurs) {

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
  while (count < nbcouleurs) {
    char *couleur = data[count];
    fprintf(p, "0 0 10 %d %d 0x%s\n", (count - 1) * slice, count * slice,
            couleur + 1);
    count++;
  }
  fprintf(p, "e\n");
  printf("Plot: FIN\n");
  pclose(p);
}

/* renvoie un message entré par le serveur
 */
int renvoie_message(json_msg *data, json_msg *reponse) {
  char message[1000];
  memset(message, 0, sizeof(message));
  printf("Votre réponse (max %ld caracteres): ", sizeof(message));
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  message[strlen(message) - 1] = '\0';
  reponse->size = data->size;
  reponse->valeurs.str_array = malloc(sizeof(char *));
  reponse->valeurs.str_array[0] = malloc(sizeof(char) * (strlen(message) + 1));
  memcpy(reponse->valeurs.str_array[0], message, strlen(message) + 1);
  return 0;
}
// Recopie le nom recu dans la réponse
int renvoie_nom(json_msg *data, json_msg *reponse) {
  reponse->size = data->size;
  reponse->valeurs.str_array = data->valeurs.str_array;
  data->valeurs.str_array = NULL;
  return 0;
}
// Fonction abstraite qui parcours les données et
// qui calcul le résultat cumulé de l'opération sur le set
double calcul(double(fct)(double, double), double *operands,
              unsigned int size) {
  double acc = operands[0];
  for (size_t i = 1; i < size; i++) {
    acc = fct(acc, operands[i]);
  }
  return acc;
}

double add(double operand1, double operand2) { return operand1 + operand2; }
double sub(double operand1, double operand2) { return operand1 - operand2; }
double mul(double operand1, double operand2) { return operand1 * operand2; }
double divide(double operand1, double operand2) {
  if (operand2 == 0.0) {
    // Ne peut pas normalement arriver ici
    perror("Erreur division par zero\n");
    return EXIT_FAILURE;
  } else {
    return operand1 / operand2;
  }
}
double min(double operand1, double operand2) {
  return operand1 <= operand2 ? operand1 : operand2;
}
double max(double operand1, double operand2) {
  return operand1 >= operand2 ? operand1 : operand2;
}
double avg(double *operands, unsigned int size) {
  return divide(calcul(add, operands, size), size);
}
double absl(double d) { return (d < 0) ? -d : d; }
// Calcule de la racine avec la méthode de Newton
double sqrt(double x) {
  double prec = __DBL_MIN__;
  double acc = 1.0;
  while (absl(acc * acc - x) >= prec) {
    acc = (x / acc + acc) / 2.0;
  }
  return acc;
}

// Calcule l'écart-type des données
double avg_diff_sqrt(double *operands, unsigned int size) {
  double xavg = avg(operands, size);
  double diffs[size];
  for (size_t i = 0; i < size; i++) {
    // Distance à la moyenne
    double s = sub(xavg, operands[i]);
    // Distance au carré divisé pas le total
    diffs[i] = divide(s * s, size);
  }
  // Somme de ces distances moyennes (variance)
  double var = calcul(add, diffs, size);
  printf("var = %lf\n", var);
  // ecart-type
  return var > 0 ? sqrt(var) : 0;
}

// Calcule et copie le résultat dans la réponse
int recois_numeros_calcule(json_msg *data, json_msg *reponse) {
  for (size_t i = 0; i < data->size; i++) {
    printf("%lf  |  ", data->valeurs.double_values->num_array[i]);
  }
  printf("\n");
  double *nums = data->valeurs.double_values->num_array;
  char *ope = data->valeurs.double_values->operateur;
  double res;
  size_t len = 8, i;
  double (*basic_fct[6])(double, double) = {add, sub, mul, divide, min, max};
  double (*fct[2])(double *, unsigned int) = {avg, avg_diff_sqrt};
  for (i = 0; i < len; i++) {
    printf("op[%ld] : %s == %s ?\n", i, ope, operateurs[i]);
    if (strcmp(ope, operateurs[i]) == 0) {
      break;
    }
  }
  if (i == 8) {
    perror("Erreur operateur");
    return EXIT_FAILURE;
  }
  res = i < 6 ? calcul(basic_fct[i], nums, data->size)
              : fct[i - 6](nums, data->size);
  printf("res : %lf\n", res);
  reponse->size = 1;
  reponse->valeurs.double_values = malloc(sizeof(calcule));
  reponse->valeurs.double_values->num_array = malloc(sizeof(double));
  reponse->valeurs.double_values->operateur = NULL;
  reponse->valeurs.double_values->num_array[0] = res;

  return 0;
}

// Plot les couleurs et les save dans un fichiers
int recois_couleurs(json_msg *data, json_msg *reponse) {
  char *rep_str;
  if (data->size > 0) {
    char file_name[30];
    sprintf(file_name, "files/%s", data->code);
    printf("%s\n", file_name);
    if (save(file_name, data))
      rep_str = "erreur save\0";
    plot(data->valeurs.str_array, data->size);
    rep_str = "enregistré\0";
  } else {
    rep_str = "erreur nombre couleurs\0";
  }
  reponse->size = 1;
  reponse->valeurs.str_array = malloc(sizeof(char *));
  reponse->valeurs.str_array[0] = malloc(sizeof(char) * (strlen(rep_str) + 1));
  memcpy(reponse->valeurs.str_array[0], rep_str, strlen(rep_str) + 1);

  return 0;
}

// Save les balises
int recois_balises(json_msg *data, json_msg *reponse) {
  char *rep_str;
  if (data->size > 0) {
    char file_name[30];
    sprintf(file_name, "files/%s", data->code);
    printf("%s\n", file_name);
    if (save(file_name, data))
      rep_str = "erreur save\0";
    else
      rep_str = "enregistré\0";
  } else {
    rep_str = "erreur nombre balises\0";
  }
  reponse->size = 1;
  reponse->valeurs.str_array = malloc(sizeof(char *));
  reponse->valeurs.str_array[0] = malloc(sizeof(char) * (strlen(rep_str) + 1));
  memcpy(reponse->valeurs.str_array[0], rep_str, strlen(rep_str) + 1);

  return 0;
}

// TODO fermer toutes les fenêtres gnuplot
void sighandler(int sigint) {
  if (is_running == 0)
    return;
  is_running = 0;
}

int main() {

  int socketfd, bind_status;
  socklen_t client_addr_len;

  struct sockaddr_in server_addr, client_addr;

  // Register SIGINT et le trap avec la fonction associée
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

  fd_set active_fd_set, read_fd_set;
  FD_ZERO(&active_fd_set); // set fd_set to zeros
  // on ajoute le socket du serveur pour le connexion entrante
  FD_SET(socketfd, &active_fd_set);
  is_running = 1;
  while (is_running == 1) {
    printf("En attente de client\n");

    // copie du set général car select est destructeur
    read_fd_set = active_fd_set;
    int read_size = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
    printf("read_size : %d\n", read_size);
    if (read_size < 0) {
      perror("erreur select");
      return EXIT_FAILURE;
    }
    // On itère sur les fd avec inputs
    for (int i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, &read_fd_set)) {
        // nouvelle connexion au serveur
        if (i == socketfd) {
          int client_socket_fd = accept(
              socketfd, (struct sockaddr *)&client_addr, &client_addr_len);
          if (client_socket_fd < 0) {
            perror("accept");
            return (EXIT_FAILURE);
          }
          printf("[+]Client %d est connecté\n", client_socket_fd);
          // on ajoute le new fd au set
          FD_SET(client_socket_fd, &active_fd_set);
        } else {
          printf("fd : %d a des choses a nous dire\n", i);
          // Lire et répondre au client
          recois_envoie_message(i);
          // fermer le socket
          close(i);
          // On enlève ce client du prochain set copié
          FD_CLR(i, &active_fd_set);
          printf("[-]Client %d déconnecté\n", i);
        }
      }
    }
  }
  close(socketfd);

  return 0;
}
