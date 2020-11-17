#include "json.h"

// Met en forme
int to_json(json_msg *json) { return 0; }

// TODO validate format before filling struct
int parse_json(char *string_json, json_msg *json) {

  char *saveptr = NULL, *saveptr_array = NULL;
  // isolé "code":"code_value"
  char *token = strtok_r(string_json, ",", &saveptr);
  // isolé "code_value"
  char *subtoken = strtok_r(NULL, ":", &token);
  strcpy(json->code, token + 1);
  printf("json code = %s\n", json->code);
  // isolé "valeurs_value"
  saveptr[strlen(saveptr) - 1] = '\0';
  token = strtok_r(NULL, ":", &saveptr);
  char *valeurs = saveptr + 2;
  valeurs[strlen(valeurs) - 1] = '\0';
  printf("save = %s  - %ld\n", valeurs, strlen(valeurs));
  // remplir valeurs[][]
  int i = 0;
  char *token_array = strtok_r(valeurs, ",", &saveptr_array);
  while (token_array != NULL) {
    strncpy(json->valeurs[i], token_array + 1, strlen(token_array) - 2);
    printf("%s  -  %ld\n", json->valeurs[i], strlen(json->valeurs[i]));
    token_array = strtok_r(NULL, ",", &saveptr_array);
    ++i;
  }

  return 0;
}

int main(int argc, char **argv) {

  json_msg json, *json_prt;
  memset(&json, 0, sizeof(json_msg));
  json_prt = &json;
  char test[100] =
      "{\"code\" : \"couleur\",\n\"valeur\" : [\"#ffffff\",\"#012345\"]}";
  parse_json(test, json_prt);
  return 0;
}