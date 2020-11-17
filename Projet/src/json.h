
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO mettre en dynamique ?
typedef struct {
  char code[10];
  char valeurs[30][7];
} json_msg;

// Met en forme
int to_json(json_msg *json);

// Lit la chaine json et rempli/crée (cf dynamic) la struc json_msg
int parse_json(char *string_json, json_msg *json);