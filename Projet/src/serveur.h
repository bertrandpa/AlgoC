/*
 * SPDX-FileCopyrightText: 2020 John Samuel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#define PORT 8089

#include "json.h"

void plot(char **data, int nbcouleurs);
int renvoie_message(json_msg *data, json_msg *reponse);
int renvoie_nom(json_msg *data, json_msg *reponse);
int recois_numeros_calcule(json_msg *data, json_msg *reponse);
int recois_couleurs(json_msg *data, json_msg *reponse);
int recois_balises(json_msg *data, json_msg *reponse);

/* accepter la nouvelle connection d'un client et lire les données
 * envoyées par le client. En suite, le serveur envoie un message
 * en retour
 */
void *recois_envoie_message(void *socketfd);

#endif
