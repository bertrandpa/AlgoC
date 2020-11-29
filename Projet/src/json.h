
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 30

typedef struct {
  char code[10];
  char valeurs[512][512];
} json_msg;

// TODO DYN
/* typedef struct {
  char *code;
  uint size;
  union {
    char **string_values;
    calcule *double_values;
  } valeurs;
} json_msg;

typedef struct {
  double *double_values;
  char *operateur;
} calcule; */

// Met en forme DYN
int to_json(char *string, json_msg *json);

// Lit la chaine json et rempli/crée (cf dynamic) la struc json_msg DYN
int parse_json(char *string_json, json_msg *json);