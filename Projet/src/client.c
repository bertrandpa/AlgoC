/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <ctype.h>
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

  char data[1024];
  json_msg json_string;

  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
  memset(&json_string, 0, sizeof(json_string));
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
    memcpy(json_string.code, "message", 8);
    // set le char valeurs dans la structure
    if (envoie_message(socketfd, &json_string)) {
      return (EXIT_SUCCESS);
    }
    break;
  case 2:
    // strcpy(data, "nom: ");
    memcpy(json_string.code, "nom", 5);
    if (envoie_nom_de_client(socketfd, &json_string)) {
      return (EXIT_SUCCESS);
    }

    break;
  case 3:
    memcpy(json_string.code, "calcule", 9);
    if (envoie_operateur_numeros(socketfd, &json_string)) {
      return (EXIT_SUCCESS);
    }
    break;
  case 4:
    memcpy(json_string.code, "couleurs", 10);
    if (envoie_couleurs(socketfd, &json_string, pathname)) {
      return (EXIT_SUCCESS);
    }
    break;
  case 5:
    memcpy(json_string.code, "balises", 9);
    if (envoie_balises(socketfd, &json_string)) {
      return (EXIT_SUCCESS);
    }

    break;
    return (EXIT_SUCCESS);
  default:
    return (EXIT_SUCCESS);
  }
  // utilise la structure pour convertir
  // les info en JSON et envoyer au serveur
  to_json(data, &json_string);
  data[strlen(data)] = '\0';

  printf("\n[send] %s\n", data);
  int write_status = write(socketfd, data, strlen(data));
  if (write_status <= 0) {
    perror("erreur ecriture");
    return (EXIT_FAILURE);
  }
  memset(data, 0, sizeof(data));
  // lire les données de la socket
  int read_status = read(socketfd, data, sizeof(data));
  if (read_status <= 0) {
    perror("erreur lecture");
    return (EXIT_FAILURE);
  }

  printf("[recv] %s\n", data);

  return 0;
}

int envoie_message(int socketfd, json_msg *data) {
  char message[100];
  memset(message, 0, sizeof(message));
  // Demandez à l'utilisateur d'entrer un message
  printf("Votre message (max 100 caracteres): ");
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  message[strlen(message) - 1] = '\0';
  strcpy(data->valeurs[0], message);
  strcpy(data->valeurs[1], "END\0");
  return 0;
}

int envoie_nom_de_client(int socketfd, json_msg *data) {
  // Demandez à l'utilisateur d'entrer un message
  char nom[40];
  memset(nom, 0, sizeof(nom));

  printf("Votre nom (max 20 caracteres): ");
  if (fgets(nom, sizeof(nom), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  nom[strlen(nom) - 1] = '\0';

  strcpy(data->valeurs[0], nom);
  strcpy(data->valeurs[1], "END\0");
  return 0;
}

int envoie_operateur_numeros(int socketfd, json_msg *data) {
  // Demandez à l'utilisateur d'entrer un message
  char calcule[1000];
  memset(calcule, 0, sizeof(calcule));
  printf("Votre calcule infixe (max 1000 caracteres): ");
  if (fgets(calcule, sizeof(calcule), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  // TODO analsye data
  char operator[10], spare[10];
  float operand1, operand2;
  int nbread =
      sscanf(calcule, "%s %f %f %s", operator, &operand1, &operand2, spare);
  if (nbread < 2 || strlen(spare) != 0) {
    perror("Erreur format");
    return (EXIT_FAILURE);
  }
  calcule[strlen(calcule) - 1] = '\0';
  strcpy(data->valeurs[0], operator);
  sprintf(data->valeurs[1], "%f", operand1);
  sprintf(data->valeurs[2], "%f", operand2);
  strcpy(data->valeurs[3], "END\0");

  return 0;
}

int analyse(char *pathname, json_msg *data, int nbcouleurs) {
  // compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);
  if (cc == NULL)
    return (EXIT_FAILURE);

  int count;
  char temp_string[10]; // care JSON change
  /* if (cc->size < 10) {  //?? size = taille_image / {3|4}
    sprintf(temp_string, "%d,", cc->size);
  } */

  for (count = 1; count < nbcouleurs + 1 && cc->size - count > 0; count++) {
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
    strcpy(data->valeurs[count], temp_string);
  }
  return 0;
}

int envoie_couleurs(int socketfd, json_msg *data, char *pathname) {
  char couleurs[1000];
  memset(couleurs, 0, sizeof(couleurs));
  uint nbcouleurs = 0;
  // char c_nbcouleurs[20];
  if (pathname != NULL) {
    do {
      printf("Votre nombre de couleurs à envoyer (<30): ");
      scanf("%u", &nbcouleurs);
    } while (nbcouleurs > 30);
    if (analyse(pathname, data, nbcouleurs))
      return (EXIT_FAILURE);
  } else {
    nbcouleurs = read_input(data, iscouleurs);
    }
  strcpy(data->valeurs[nbcouleurs + 1], "END\0");
  sprintf(data->valeurs[0], "%u", nbcouleurs);
  return 0;
}

int envoie_balises(int socketfd, json_msg *data) {

  uint nbbalises = read_input(data, isbalises);
  sprintf(data->valeurs[0], "%u", nbbalises);
  strcpy(data->valeurs[nbbalises + 1], "END\0");
  /* char res[300];
  sprintf(res, "%d", nbbalises);
  strcat(res, balises);
  strcat(data->valeurs[0], res); */
  return 0;
}

int iscouleurs(char *couleur) {
  uint32_t hexa_color;
  if (sscanf(couleur, "%x", &hexa_color) == 1) {
    // test 6 digit hexa
    if (strlen(couleur) == 6) {
      return 0;
    } else {
      return 1;
    }
  } else {
    return -1;
  }
}

int isbalises(char *balise) {
  for (int i = 0; i < strlen(balise); i++) {
    if (!isalnum(balise[i]))
      return -1;
    else if (!isalpha(balise[i]))
      return 1;
  }
  return 0;
}

int read_input(json_msg *data, int(test)(char *)) {
  uint count = 0;
  char tmp_str[30];
  do {
    memset(tmp_str, 0, sizeof(tmp_str));
    printf("Votre input : ");
    if (fgets(tmp_str, sizeof(tmp_str), stdin) == NULL) {
      perror("erreur scan utilisateur");
      return (EXIT_FAILURE);
    }
    tmp_str[strlen(tmp_str) - 1] = '\0';
    int format = test(tmp_str);
    if (format == 0) {
      sprintf(data->valeurs[count + 1], "#%s", tmp_str);
      count++;
    } else if (format == 1) {
      printf("Erreur format\n");
    } else {
      printf("\nFin de la saisie\n");
      break;
    }
  } while (count < 30);
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
