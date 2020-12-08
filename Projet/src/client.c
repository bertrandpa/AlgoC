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

int envoie_recois_message(int socketfd, char *pathname) {

  char json_string[1024];
  json_msg *json = malloc(sizeof(json_msg));

  // la réinitialisation de l'ensemble des données
  memset(json_string, 0, sizeof(json_string));
  memset(json, 0, sizeof(*json));
  // memset(&json, 0, sizeof(json));
  int choice;
  printf(
      "Choisir un mode d'envoie :\n\tmessage:  1\n\tnom:      2\n\tcalcule:  "
      "3\n\tcouleurs: 4\n\tbalises:  5\nEntrez le numéro "
      "correspondant :");
  char tmp[10];
  if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  if (sscanf(tmp, "%d", &choice) != 1)
    return (EXIT_FAILURE);
  switch (choice) {
  case 1:
    // set le char code dans la structure
    memcpy(json->code, "message", 8);
    json->size = 1;
    // allocation d'un array de 1 char *
    json->valeurs.str_array = calloc(json->size, sizeof(char *));
    // set le char valeurs dans la structure
    if (envoie_message(json)) {
      delete_json(json);
      return (EXIT_FAILURE);
    }
    break;
  case 2:
    memcpy(json->code, "nom", 5);
    json->size = 1;
    json->valeurs.str_array = calloc(json->size, sizeof(char *));
    printf("after allocation\n");
    if (envoie_nom_de_client(json)) {
      delete_json(json);
      return (EXIT_FAILURE);
    }
    break;
  case 3:
    memcpy(json->code, "calcule", 9);
    if (envoie_operateur_numeros(json)) {
      return (EXIT_FAILURE);
    }
    break;
  case 4:
    memcpy(json->code, "couleurs", 10);
    if (envoie_couleurs(json, pathname)) {
      delete_json(json);
      return (EXIT_FAILURE);
    }
    break;
  case 5:
    memcpy(json->code, "balises", 9);
    if (envoie_balises(json)) {
      delete_json(json);
      return (EXIT_FAILURE);
    }
    break;
  default:
    delete_json(json);
    return (EXIT_FAILURE);
  }
  // utilise la structure pour convertir
  // les info en JSON et envoyer au serveur

  to_json(json_string, json);

  json_string[strlen(json_string)] = '\0';

  int write_status = write(socketfd, json_string, strlen(json_string));
  printf("\n[send] %s\n", json_string);
  delete_json(json);
  if (write_status <= 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }
  memset(json_string, 0, sizeof(json_string));
  // lire les données de la socket
  int read_status = read(socketfd, json_string, sizeof(json_string));
  if (read_status <= 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  }

  printf("[recv] %s\n", json_string);

  return 0;
}

int envoie_message(json_msg *data) {
  char message[100];
  memset(message, 0, sizeof(message));
  // Demandez à l'utilisateur d'entrer un message
  printf("Votre message (max 100 caracteres): ");
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  message[strlen(message) - 1] = '\0';
  data->valeurs.str_array[0] = malloc(sizeof(char) * (strlen(message) + 1));
  memcpy(data->valeurs.str_array[0], message, strlen(message) + 1);
  return 0;
}

int envoie_nom_de_client(json_msg *data) {
  // Demandez à l'utilisateur d'entrer un message
  char nom[40];
  memset(nom, 0, sizeof(nom));

  printf("Votre nom (max 20 caracteres): ");
  if (fgets(nom, sizeof(nom), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  nom[strlen(nom) - 1] = '\0';
  data->valeurs.str_array[0] = malloc(sizeof(char) * (strlen(nom) + 1));
  memcpy(data->valeurs.str_array[0], nom, strlen(nom) + 1);
  return 0;
}

int envoie_operateur_numeros(json_msg *data) {
  char ope[20];
  memset(ope, 0, sizeof(ope));
  printf("Votre opération (%s, %s, %s, %s, %s, %s, %s, %s): ", operateurs[0],
         operateurs[1], operateurs[2], operateurs[3], operateurs[4],
         operateurs[5], operateurs[6], operateurs[7]);
  if (fgets(ope, sizeof(ope), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  ope[strlen(ope) - 1] = '\0';
  int ope_code = isoperateur(ope);
  if (!ope_code) {
    perror("erreur format operateur");
    return (EXIT_FAILURE);
  }
  int max_size_array = ope_code < 5 ? 2 : MAX_INPUT;
  calcule *calc = malloc(sizeof(calcule));
  calc->operateur = malloc(sizeof(char) * (strlen(ope) + 1));
  memcpy(calc->operateur, ope, strlen(ope) + 1);
  data->valeurs.double_values = calc;
  data->size = read_number(data, max_size_array);
  return 0;
}

char **analyse(char *pathname, int nbcouleurs) {
  // compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);
  char **couleurs = malloc(sizeof(char *) * nbcouleurs);
  if (cc == NULL)
    return NULL;

  int count;
  char temp_string[10];

  for (count = 0; count < nbcouleurs && cc->size - count > 0; count++) {
    if (cc->compte_bit == BITS32) {
      sprintf(temp_string, "#%02x%02x%02x",
              cc->cc.cc32[cc->size - count].c.rouge,
              cc->cc.cc32[cc->size - count].c.vert,
              cc->cc.cc32[cc->size - count].c.bleu);
    }
    if (cc->compte_bit == BITS24) {
      sprintf(temp_string, "#%02x%02x%02x",
              cc->cc.cc24[cc->size - count].c.rouge,
              cc->cc.cc24[cc->size - count].c.vert,
              cc->cc.cc24[cc->size - count].c.bleu);
    }
    printf(" ( %s\n", temp_string);
    couleurs[count] = malloc(sizeof(char) * (strlen(temp_string) + 1));
    memcpy(couleurs[count], temp_string, strlen(temp_string) + 1);
    printf(" %s )\n", couleurs[count]);
  }
  // Pour éviter plus d'1Mo de fuite mémoire non géré initialement
  delete_couleur_compteur(cc);
  return couleurs;
}

int envoie_couleurs(json_msg *data, char *pathname) {
  char couleurs[1000];
  memset(couleurs, 0, sizeof(couleurs));
  uint nbcouleurs = 0;
  if (pathname != NULL) {
    do {
      printf("Votre nombre de couleurs à envoyer (<30): ");
      scanf("%u", &nbcouleurs);
    } while (nbcouleurs > 30 && nbcouleurs < 1);
    char **arr = analyse(pathname, nbcouleurs);
    printf("%s, %ld\n", arr[0], strlen(*arr));
    data->size = nbcouleurs;
    data->valeurs.str_array = arr;

  } else {
    data->size = read_string(data, iscouleurs);
  }
  if (data->valeurs.str_array == NULL)
    return (EXIT_FAILURE);
  return 0;
}

int envoie_balises(json_msg *data) {
  data->size = read_string(data, isbalises);
  printf("size read : %ld\n", sizeof(data->valeurs.str_array));
  if (data->valeurs.str_array == NULL)
    return (EXIT_FAILURE);
  return 0;
}

int read_string(json_msg *data, int(test)(char *)) {
  uint count = 0;
  char tmp_str[50];
  char **arr = malloc(MAX_INPUT * sizeof(char *));
  do {
    memset(tmp_str, 0, sizeof(tmp_str));
    printf("Votre input n°%d (& pour arreter la saisie): ", count + 1);
    if (fgets(tmp_str, sizeof(tmp_str), stdin) == NULL) {
      perror("erreur scan utilisateur");
      return (EXIT_FAILURE);
    }
    tmp_str[strlen(tmp_str) - 1] = '\0';
    int format = test(tmp_str);
    if (format == 0) {
      arr[count] = malloc(strlen(tmp_str) + 2);
      sprintf(arr[count], "#%s", tmp_str);
      // printf("arr[%d] = %s, size : %ld\n", count, arr[count],
      // strlen(arr[count]));
      count++;
    } else if (strcmp(tmp_str, "&") == 0) {
      printf("\nFin de la saisie\n");
      break;
    } else {
      printf("Erreur format\n");
    }
  } while (count < MAX_INPUT);
  arr = realloc(arr, sizeof(char *) * count);
  data->valeurs.str_array = arr;
  return count;
}
int read_number(json_msg *data, unsigned int max_op) {

  uint count = 0;
  char tmp_str[50];
  double *arr = malloc(max_op * sizeof(char *));
  do {
    memset(tmp_str, 0, sizeof(tmp_str));
    printf("Votre input n°%d (& pour arreter la saisie): ", count + 1);
    if (fgets(tmp_str, sizeof(tmp_str), stdin) == NULL) {
      perror("erreur scan utilisateur");
      return (EXIT_FAILURE);
    }
    tmp_str[strlen(tmp_str) - 1] = '\0';
    int format = isnumber(tmp_str);
    if (format == 0) {
      arr[count] = atof(tmp_str);
      // printf("arr[%d] = %lf\n", count, arr[count]);
      count++;
    } else if (strcmp(tmp_str, "&") == 0) {
      printf("\nFin de la saisie\n");
      break;
    } else {
      printf("Erreur format\n");
    }
  } while (count < max_op);
  arr = realloc(arr, sizeof(double) * count);
  data->valeurs.double_values->num_array = arr;
  return count;
}

int main(int argc, char **argv) {
  int socketfd;

  struct sockaddr_in server_addr;

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

  envoie_recois_message(socketfd, argv[1]);

  close(socketfd);
}
