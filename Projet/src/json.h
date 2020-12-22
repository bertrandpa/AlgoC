#ifndef __JSON_H__
#define __JSON_H__

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 30

typedef struct {
  double *num_array;
  char *operateur;
} calcule;

typedef struct {
  char code[10];
  unsigned int size;
  union {
    char **str_array;
    calcule *double_values;
  } valeurs;
} json_msg;

static const char codes[5][12] = {"\"message\"\0", "\"nom\"\0", "\"calcule\"\0",
                                  "\"couleurs\"\0", "\"balises\"\0"};
static const char operateurs[8][6] = {"+\0",   "-\0",   "*\0",   "/\0",
                                      "min\0", "max\0", "avg\0", "ect\0"};

// Test couleur valide
int iscouleurs(char *couleur);

// Test format balise
int isbalises(char *balise);

/** Test si l'opérateur existe dans le protocole de calcule
 * retourne son (index+1) dans le tableau operateurs
 * 0 sinon
 */
int isoperateur(char *operateur);

// Test si le char est un double
int isnumber(char *number);

/** Test si le code existe dans le protocole
 * retourne son (index+1) dans le tableau codes
 * 0 sinon
 */
int iscode(char *code);

// Test si la chaîne est entre guillemets
int isquoted(char *str);

// Valide une structure calcule
int validate_calcul(calcule *calc, unsigned int *arr_size);

// Transforme la struct json en un string au format json
int json_to_string(char *string, json_msg *json);

// Fonction qui enlève les espaces, sauf dans les chaines entre guillemets
char *trim(char *src);

// Lit la chaine json, valide le format et rempli la struct json
int parse_json(char *string_json, json_msg *json);

void delete (void *ptr, char *name);
// Fonction delete de la struct
void delete_json(json_msg *json);

#endif