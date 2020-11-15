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
    strcpy(data, "couleurs: ");
    envoie_couleurs(socketfd, data, pathname);
    break;
  case 5:
    // envoie_balises(socketfd, data) //TODO
    // break;
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
  // Demandez à l'utilisateur d'entrer un message
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
  // Demandez à l'utilisateur d'entrer un message
  char calcule[1000];
  memset(calcule, 0, sizeof(calcule));
  printf("Votre calcule infixe (max 1000 caracteres): ");
  if (fgets(calcule, sizeof(calcule), stdin) == NULL) {
    perror("erreur scan utilisateur");
    return (EXIT_FAILURE);
  }
  // TODO analsye data
  char operator, spare[10];
  float operand1, operand2;
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

void analyse(char *pathname, char *data, int nbcouleurs) {
  // compte de couleurs
  couleur_compteur *cc = analyse_bmp_image(pathname);

  int count;
  char temp_string[10]; // care JSON change
  /* if (cc->size < 10) {  //?? size = taille_image / {3|4}
    sprintf(temp_string, "%d,", cc->size);
  } */

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
}

int envoie_couleurs(int socketfd, char *data, char *pathname) {
  char couleurs[1000];
  memset(couleurs, 0, sizeof(couleurs));
  uint nbcouleurs = 0;
  if (pathname != NULL) {
    do {
      printf("Votre nombre de couleurs à envoyer (<30): ");
      scanf("%u", &nbcouleurs);
      printf("%u\n", nbcouleurs);
    } while (nbcouleurs > 30);
    analyse(pathname, couleurs, nbcouleurs);
  } else {
    char tmp_couleur[10];
    do {
      memset(tmp_couleur, 0, sizeof(tmp_couleur));
      uint32_t couleur;
      printf("Votre couleur (format hexa): ");
      if (fgets(tmp_couleur, sizeof(tmp_couleur), stdin) == NULL) {
        perror("erreur scan utilisateur");
        return (EXIT_FAILURE);
      }
      // vérifie le format hexa
      if (sscanf(tmp_couleur, "%x", &couleur) == 1) {
        // remplace le \n du fgets par '\0'
        tmp_couleur[strlen(tmp_couleur) - 1] = '\0';
        // test 6 digit hexa
        if (strlen(tmp_couleur) == 6) {
          nbcouleurs++;
          strcat(couleurs, ",#");
          strcat(couleurs, tmp_couleur);
        } else {
          printf("Erreur format\n");
        }

      } else {
        printf("\nFin de la saisie\n");
        break;
      }
    } while (nbcouleurs < 31);
  }
  sprintf(data, "couleurs: %d", nbcouleurs);
  strcat(data, couleurs);
  return 0;
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
  // TODO arbre choix ?
  envoie_recois_message(socketfd, argv[1]);

  close(socketfd);
}
