#include "json.h"

// TODO validate format before filling struct
int parse_json(char *string_json, json_msg *json) {

  char *saveptr = NULL, *saveptr_array = NULL;
  // isolé "code":"code_value"
  char *token = strtok_r(string_json, ",", &saveptr);
  printf("token : %s\nsaveptr : %s\n", token, saveptr);
  // isolé "code_value"
  char *subtoken = strtok_r(NULL, ":", &token);
  printf("subtoken : %s\ntoken : %s\n", subtoken, token);
  strncpy(json->code, token + 2, strlen(token) - 3);
  json->code[strlen(json->code)] = '\0';
  // isolé "valeurs_value"
  saveptr[strlen(saveptr) - 1] = '\0';
  token = strtok_r(NULL, ":", &saveptr);
  printf("token : %s\nsaveptr : %s\n", token, saveptr);
  char *valeurs = saveptr + 2;
  valeurs[strlen(valeurs) - 3] = '\0';
  printf("valeurs : %s\n", valeurs);
  unsigned int i = 0;
  char *token_array = strtok_r(valeurs, ", ", &saveptr_array);
  printf("token_array : %s\nsaveptr : %s\n", token_array, saveptr_array);
  // TODO parametriser
  json->valeurs.str_array = calloc(30, sizeof(char *));
  while (token_array != NULL) {
    // TODO fix offset if quotes + param
    // int len = strlen(token_array) - 2;
    token_array += 1;
    token_array[strlen(token_array) - 1] = '\0';
    json->valeurs.str_array[i] =
        malloc(sizeof(char) * (strlen(token_array) + 1));
    memcpy(json->valeurs.str_array[i], token_array, strlen(token_array) + 1);
    printf("valeurs [%d] : %s\n", i, json->valeurs.str_array[i]);
    token_array = strtok_r(NULL, ", ", &saveptr_array);
    ++i;
  }
  json->size = i;
  json->valeurs.str_array =
      realloc(json->valeurs.str_array, i * sizeof(char *));
  return 0;
}

void remove_zeros(char *str_double) {
  for (size_t i = strlen(str_double) - 1; i > 1 && str_double[i - 1] != '.';
       i--) {
    if (str_double[i] == '0')
      str_double[i] = '\0';
  }
}

void append_calcule(char *string, json_msg *json) {
  double *arr = json->valeurs.double_values->num_array;
  char *ope = json->valeurs.double_values->operateur;
  char tmpstr[50];

  size_t i = 0;
  snprintf(string, strlen(ope) + 2, " %s,", ope);
  for (i = 0; i < json->size - 1; i++) {
    sprintf(tmpstr, "%f", arr[i]);
    remove_zeros(tmpstr);
    snprintf(string, strlen(tmpstr) + 2, " %s,", tmpstr);
  }
  sprintf(tmpstr, "%f", arr[i]);
  remove_zeros(tmpstr);
  snprintf(string, strlen(tmpstr) + 2, " %s ", tmpstr);
}

// Met en forme
int to_json(char *string, json_msg *json) {
  sprintf(string, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [", json->code);
  printf("%s\n", string);
  if (strcmp(json->code, "calcule") == 0)
    append_calcule(string, json);
  else {
    char **arr = json->valeurs.str_array;
    if (json->size > 1 && (strcmp(json->code, "couleurs") == 0 ||
                           strcmp(json->code, "balises") == 0)) {
      sprintf(string + strlen(string), " %d,", json->size);
    }
    char tmpstr[2048];
    unsigned int i;
    for (i = 0; i < json->size - 1; i++) {
      memset(tmpstr, 0, sizeof(tmpstr));
      snprintf(string + strlen(string), strlen(arr[i]) + 5, " \"%s\",", arr[i]);
    }
    /* sprintf(tmpstr, " \"%s\" ", arr[i]);
    strcat(string, tmpstr); */
    snprintf(string + strlen(string), strlen(arr[i]) + 5, " \"%s\" ", arr[i]);
  }
  strcat(string, "]\n}\0");
  printf("%s\n", string);
  return 0;
}

void delete_s(void *ptr, char *name) {
  ptr != NULL ? free(ptr) : printf("cannot free %s\n", name);
}

void delete_json(json_msg *json) {
  if (json != NULL) {
    if (strcmp(json->code, "calcule") == 0) {
      if (json->valeurs.double_values != NULL) {
        delete_s(json->valeurs.double_values->num_array, "num_array");
        delete_s(json->valeurs.double_values->operateur, "operateur");
      }
    } else {
      if (json->valeurs.str_array != NULL) {
        for (uint i = 0; i < json->size; i++) {
          // TODO fix
          // printf("arr[%d] sizeof : %ld\n", i,
          // sizeof(json->valeurs.str_array));
          delete_s(json->valeurs.str_array[i], "string_array value");
        }
        delete_s(json->valeurs.str_array, "string_array");
      }
    }
  }
  delete_s(json, "json");
}