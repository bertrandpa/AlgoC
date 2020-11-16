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
  // la réinitialisation de l'ensemble des données
  memset(data, 0, sizeof(data));
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
    strcpy(data, "message: ");
    envoie_message(socketfd, data);
    break;
  case 2:
    strcpy(data, "nom: ");
    envoie_nom_de_client(socketfd, data);
    break;
  case 3:
    strcpy(data, "calcule: ");
    envoie_operateur_numeros(socketfd, data);
    break;
  case 4:
    // TODO soit demandé l'input du path
    // soit si pathname != null call direct la fct
    strcpy(data, "couleurs: ");
    envoie_couleurs(socketfd, data, pathname);
    break;
  case 5:
    envoie_balises(socketfd, data);
    break;
    return (EXIT_SUCCESS);
  default:
    return (EXIT_SUCCESS);
  }

  // printf("\n[send] %s\n", data);
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

int envoie_message(int socketfd, char *data) {
  char message[100];
  memset(message, 0, sizeof(message));
  // Demandez à l'utilisateur d'entrer un message
  printf("Votre message (max 100 caracteres): ");
  if (fgets(message, sizeof(message), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  message[strlen(message) - 1] = '\0';
  strcat(data, message);
  return 0;
}

int envoie_nom_de_client(int socketfd, char *data) {
  char nom[40];
  memset(nom, 0, sizeof(nom));
  printf("Votre nom (max 20 caracteres): ");
  if (fgets(nom, sizeof(nom), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  strcat(data, nom);
  return 0;
}

int envoie_operateur_numeros(int socketfd, char *data) {
  char calcule[1000];
  memset(calcule, 0, sizeof(calcule));
  printf("Votre calcule infixe (max 1000 caracteres): ");
  if (fgets(calcule, sizeof(calcule), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  char operator, spare[10];
  float operand1, operand2;
  // TODO refactor pour JSON (fgets + parse)
  int nbread =
      sscanf(calcule, "%c %f %f %s", &operator, &operand1, &operand2, spare);
  if (nbread != 3 || strlen(spare) != 0) {
    perror("Erreur format");
    return (EXIT_FAILURE);
  }
  strcpy(data, "calcule: ");
  strcat(data, calcule);
  return 0;
}

int analyse(char *pathname, char *data, int nbcouleurs) {
  // compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);
  if (cc == NULL)
    return (EXIT_FAILURE);

  int count;
  char temp_string[10]; // care JSON change

  for (count = 1; count < nbcouleurs + 1 && cc->size - count > 0; count++) {
    if (cc->compte_bit == BITS32) {
      sprintf(temp_string, ",#%02x%02x%02x",
              cc->cc.cc32[cc->size - count].c.rouge,
              cc->cc.cc32[cc->size - count].c.vert,
              cc->cc.cc32[cc->size - count].c.bleu);
    }
    if (cc->compte_bit == BITS24) {
      sprintf(temp_string, ",#%02x%02x%02x",
              cc->cc.cc24[cc->size - count].c.rouge,
              cc->cc.cc24[cc->size - count].c.vert,
              cc->cc.cc24[cc->size - count].c.bleu);
    }
    strcat(data, temp_string);
  }
  return 0;
}

int envoie_couleurs(int socketfd, char *data, char *pathname) {
  char couleurs[1000];
  memset(couleurs, 0, sizeof(couleurs));
  uint nbcouleurs = 0;
  if (pathname != NULL) {
    do {
      printf("Votre nombre de couleurs à envoyer (<30): ");
      scanf("%u", &nbcouleurs);
    } while (nbcouleurs > 30);
    if (analyse(pathname, couleurs, nbcouleurs))
      return (EXIT_FAILURE);
  } else {
    // Fonction de validation du format passée en paramètre
    nbcouleurs = read_input(couleurs, iscouleurs);
  }
  sprintf(data, "couleurs: %d", nbcouleurs);
  strcat(data, couleurs);
  return 0;
}

int envoie_balises(int socketfd, char *data) {

  char balises[1000];
  memset(balises, 0, sizeof(balises));
  // Fonction de validation du format passée en paramètre
  uint nbbalises = read_input(balises, isbalises);
  sprintf(data, "balises: %d", nbbalises);
  strcat(data, balises);
  return 0;
}

// test validation format couleur
int iscouleurs(char *couleur) {
  uint32_t hexa_color;
  // test format hexa
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

// test validation format balise
int isbalises(char *balise) {
  for (int i = 0; i < strlen(balise); i++) {
    if (!isalnum(balise[i]))
      return -1;
    else if (!isalpha(balise[i]))
      return 1;
  }
  return 0;
}

// On passe la fonction de test en paramètre
int read_input(char *data, int (*test)(char *)) {
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
    // Recupère état du test format
    int format = test(tmp_str);
    // format OK
    if (format == 0) {
      strcat(data, ", #");
      strcat(data, tmp_str);
      count++;
    }
    // format erroné
    else if (format == 1) {
      printf("Erreur format\n");
    }
    // format fin
    else {
      printf("\nFin de la saisie\n");
      break;
    }
  } while (count < 31);
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
