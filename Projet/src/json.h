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

// TODO DYN
typedef struct {
  char code[10];
  unsigned int size;
  union {
    char **str_array;
    calcule *double_values;
  } valeurs;
} json_msg;

static const char codes[][12] = {"\"message\"\0", "\"nom\"\0", "\"calcule\"\0",
                                 "\"couleurs\"\0", "\"balises\"\0"};
static const char operateurs[][6] = {"\"+\"\0",   "\"-\"\0",   "\"*\"\0",
                                     "\"/\"\0",   "\"min\"\0", "\"max\"\0",
                                     "\"avg\"\0", "\"ect\"\0"};

int iscouleurs(char *couleur);
int isbalises(char *balise);
int isoperateur(char *operateur);
int isnumber(char *number);
int iscode(char *code);

// Met en forme DYN
int to_json(char *string, json_msg *json);

// Lit la chaine json et rempli/crée (cf dynamic) la struc json_msg DYN
int parse_json(char *string_json, json_msg *json);

void delete (void *ptr, char *name);
void delete_json(json_msg *json);

#endif