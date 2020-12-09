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

int iscouleurs(char *couleur);
int isbalises(char *balise);
int isoperateur(char *operateur);
int isnumber(char *number);
int iscode(char *code);

// Transforme la struct en un string au format json
int json_to_string(char *string, json_msg *json);

// Lit la chaine json, valide le format et rempli la struct json_msg
int parse_json(char *string_json, json_msg *json);

void delete (void *ptr, char *name);
// Fonction delete de la struct
void delete_json(json_msg *json);

#endif