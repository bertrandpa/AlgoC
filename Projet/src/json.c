#include "json.h"

// Met en forme

// TODO validate format before filling struct
int parse_json(char *string_json, json_msg *json) {

  char *saveptr = NULL, *saveptr_array = NULL;
  // isolé "code":"code_value"
  char *token = strtok_r(string_json, ",", &saveptr);
  // isolé "code_value"
  char *subtoken = strtok_r(NULL, ":", &token);
  strncpy(json->code, token + 2, strlen(token) - 3);
  json->code[strlen(json->code)] = '\0';
  // isolé "valeurs_value"
  saveptr[strlen(saveptr) - 1] = '\0';
  token = strtok_r(NULL, ":", &saveptr);
  char *valeurs = saveptr + 2;
  valeurs[strlen(valeurs) - 1] = '\0';
  int i = 0;
  char *token_array = strtok_r(valeurs, ",", &saveptr_array);
  while (token_array != NULL) {
    strncpy(json->valeurs[i], token_array + 2, strlen(token_array) - 3);
    printf("valeurs [%d] : %s\n", i, json->valeurs[i]);
    token_array = strtok_r(NULL, ",", &saveptr_array);
    ++i;
  }
  strcpy(json->valeurs[i], "END");

  return 0;
}

int to_json(char *string, json_msg *json) {
  sprintf(string, "{ \"code\" : \"%s\",\n\"valeurs\" : [", json->code);
  printf("%s\n", string);
  char tmpstr[2048];
  int i;
  for (i = 0; strcmp(json->valeurs[i], "END") != 0; ++i) {
    if (strcmp(json->valeurs[i + 1], "END") == 0) {
      sprintf(tmpstr, " \"%s\"", json->valeurs[i]);
    } else {
      sprintf(tmpstr, " \"%s\",", json->valeurs[i]);
    }
    strcat(string, tmpstr);
    printf("%s\n", string);
  }
  strcat(string, "]}");
}