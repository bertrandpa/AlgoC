
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 30

typedef struct {
  char code[10];
  char valeurs[512][512];
} json_msg;

// Met en forme
const char *toJson(json_msg *msg);
int to_json(char *string, json_msg *json);

// Lit la chaine json et rempli/crée (cf dynamic) la struc json_msg
int parse_json(char *string_json, json_msg *json);