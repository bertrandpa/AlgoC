#include "json.h"

int iscouleurs(char *couleur) {
  unsigned int hexa_color;
  if (sscanf(couleur, "%x", &hexa_color) == 1) {
    // test 6 digit hexa
    if (strlen(couleur) == 6) {
      return 0;
    }
  }
  return 1;
}

int isbalises(char *balise) {
  for (size_t i = 0; i < strlen(balise); i++) {
    if (!isalpha(balise[i]))
      return 1;
  }
  return 0;
}

int iscode(char *code) {
  for (size_t i = 0; i < sizeof(codes) / sizeof(*codes); i++) {
    if (strcmp(code, codes[i]) == 0)
      return i + 1;
  }
  return 0;
}

int isnumber(char *number) {
  double n;
  return !sscanf(number, "%lf", &n);
}

int isquoted(char *str) {
  if (str[0] == '\"' && str[strlen(str) - 1] == '\"')
    return 0;
  return 1;
}

int ismessage(char *message) {
  if (isquoted(message))
    return 1;
  message++;
  message[strlen(message) - 1] = '\0';
  for (size_t i = 1; i < strlen(message) - 1; i++) {
    if (!isalnum(message[i]))
      return 1;
  }
  return 0;
}
int isoperateur(char *operateur) {
  for (size_t i = 0; i < sizeof(operateurs) / sizeof(*operateurs); i++) {
    if (strcmp(operateur, operateurs[i]) == 0)
      return i + 1;
  }
  return 0;
}

int is_str_balises(char *strbalise) {
  if (isquoted(strbalise) && strbalise[1] != '#')
    return 1;
  strbalise++;
  strbalise[strlen(strbalise) - 1] = '\0';
  printf("balise cutted : %s\n", strbalise);
  return isbalises(strbalise + 1);
}

int is_str_couleurs(char *strcouleur) {
  if (isquoted(strcouleur) && strcouleur[1] != '#')
    return 1;
  strcouleur++;
  strcouleur[strlen(strcouleur) - 1] = '\0';
  // printf("couleur : %s\n", strcouleur);
  return iscouleurs(strcouleur + 1);
}

int (*str_code_test(int code))(char *) {
  if (code == 4) {
    return is_str_couleurs;
  } else if (code == 5) {
    return is_str_balises;
  } else if (code < 3) {
    return ismessage;
  }
  return NULL;
}

int validate_calcul(calcule *calc, unsigned int *arr_size) {

  int code_op = isoperateur(calc->operateur);
  if (!code_op) {
    perror("erreur format operateur");
    return EXIT_FAILURE;
  }
  if (*arr_size <= 0 || (code_op < 5 && *arr_size > 2) ||
      *arr_size > MAX_INPUT) {
    perror("erreur taille array");
    return EXIT_FAILURE;
  }
  // Si un seul carac
  if (code_op < 5 && (*arr_size) == 1) {
    (*arr_size) += 1;
    calc->num_array = realloc(calc->num_array, sizeof(double) * (*arr_size));
    calc->num_array[1] = calc->num_array[0];
    // si +,- on ajoute un 0 et si *, / on ajoute un 1
    // le tout à partir du code (fait pour que si on envoie "-" 1 on retourne -1
    // et non 1 et "/" 3 => 1/3 et non 3)
    calc->num_array[0] = (code_op + 1) >> 2;
    if (code_op == 4 && calc->num_array[1] == 0.0) {
      perror("erreur division par zero");
      return EXIT_FAILURE;
    }
  }

  return 0;
}

char *trim(char *src) {
  printf(src);
  char *res, *token, *saveptr, tmpres[3072];
  memset(tmpres, 0, sizeof(tmpres));
  char *delim = " \t\n";
  token = strtok_r(src, delim, &saveptr);
  while (token != NULL) {

    strncat(tmpres, token, strlen(token));
    token = strtok_r(NULL, delim, &saveptr);
  }
  tmpres[strlen(tmpres)] = '\0';
  res = malloc(sizeof(char) * strlen(tmpres) + 1);
  memcpy(res, tmpres, strlen(tmpres) + 1);
  return res;
}

int parse_json(char *string_json, json_msg *json) {

  char *saveptr = NULL, *saveptr_array = NULL;
  // get trimmed json
  char *tstr = trim(string_json);
  printf("trimmed : %s\n", tstr);
  // isolé "code":"code_value"
  char *token = strtok_r(tstr, ",", &saveptr);
  printf("token : %s\nsaveptr : %s\n", token, saveptr);

  // isolate and compare header
  char *subtoken = strtok_r(NULL, ":", &token);
  printf("subtoken : %s\ntoken : %s\n", subtoken, token);
  if (strcmp(subtoken, "{\"code\"") != 0) {
    perror("erreur format json 1");
    free(tstr);
    return EXIT_FAILURE;
  }
  // check code value
  int code = iscode(token);
  if (!code) {
    perror("erreur mauvais code");
    free(tstr);
    return EXIT_FAILURE;
  }
  strncpy(json->code, token + 1, strlen(token) - 2);
  json->code[strlen(token) - 1] = '\0';
  printf("code : %s\n", json->code);

  token = strtok_r(NULL, ":", &saveptr);
  printf("token : %s\nsaveptr : %s\n", token, saveptr);
  if (strcmp(token, "\"valeurs\"") != 0) {
    perror("erreur format json 2");
    free(tstr);
    return EXIT_FAILURE;
  }
  // check json footer
  if (saveptr[0] != '[' && saveptr[strlen(saveptr) - 2] != ']' &&
      saveptr[strlen(saveptr) - 1] != '}') {
    perror("erreur format json 3");
    free(tstr);
    return EXIT_FAILURE;
  }
  char *valeurs = saveptr + 1;

  valeurs[strlen(valeurs) - 2] = '\0';
  printf("valeurs : %s\n", valeurs);

  int i = 0;
  char *token_array = strtok_r(valeurs, ",", &saveptr_array);
  printf("token_array : %s\nsaveptr : %s\n", token_array, saveptr_array);

  // prepare array parse
  int pad = 1, tmp_size = 0;
  double *array = NULL;
  char **strings = NULL, *ope = NULL;
  int (*test)(char *);
  // Si code autre que message et nom
  if (code > 2) {
    // verification du format du premier element
    if (code == 3) {
      if (isquoted(token_array)) {
        perror("erreur format operateur");
        free(tstr);
        return EXIT_FAILURE;
      } else {
        token_array++;
        token_array[strlen(token_array) - 1] = '\0';
        ope = malloc(sizeof(char) * (strlen(token_array) + 1));
        memcpy(ope, token_array, strlen(token_array));
        ope[strlen(token_array)] = '\0';
        array = malloc(sizeof(double) * MAX_INPUT);
        // Fonction de validation
        test = isnumber;
        pad = 0;
      }
    } else {
      tmp_size = atoi(token_array);
      if (tmp_size <= 0) {
        perror("erreur format array");
        free(tstr);
        return EXIT_FAILURE;
      }
    }
    // On passe à l'element suivant de l'array
    token_array = strtok_r(NULL, ",", &saveptr_array);
    printf("token_array : %s\nsaveptr : %s\n", token_array, saveptr_array);
  }
  if (pad) {
    // Fonction de validation
    test = str_code_test(code);
    strings = malloc(sizeof(char *) * MAX_INPUT);
  }
  // parse array
  while (token_array != NULL) {
    printf("token [%d] : %s\n", i, token_array);
    // Validation en fonction du code
    if (test(token_array)) {
      perror("erreur format data 1");
      free(tstr);
      return EXIT_FAILURE;
    }
    if (code == 3) {
      array[i] = atof(token_array);
      printf("array [%d] : %lf\n", i, array[i]);

    } else {
      strings[i] = malloc(sizeof(char *) * (strlen(token_array) - 1));
      memcpy(strings[i], token_array + 1, strlen(token_array) - 1);
      strings[i][strlen(token_array) - 1] = '\0';
      printf("string [%d] : %s\n", i, strings[i]);
    }

    token_array = strtok_r(NULL, ",", &saveptr_array);
    ++i;
  }
  // Comparaison de la taille donnée dans le msg avec la taille réelle de
  // l'array
  if (tmp_size != 0 && i != tmp_size) {
    perror("erreur taille array");
    free(tstr);
    return EXIT_FAILURE;
  }
  json->size = i;
  if (code == 3) {
    array = realloc(array, i * sizeof(double));
    json->valeurs.double_values = malloc(sizeof(calcule));
    json->valeurs.double_values->num_array = array;
    json->valeurs.double_values->operateur = ope;
    int valid_status =
        validate_calcul(json->valeurs.double_values, &json->size);
    if (valid_status) {
      free(tstr);
      return EXIT_FAILURE;
    }
  } else {
    strings = realloc(strings, i * sizeof(char *));
    json->valeurs.str_array = strings;
  }
  free(tstr);
  return 0;
}

// Enlève les zeros en trop
void remove_zeros(char *str_double) {
  size_t i = strlen(str_double) - 1;
  while (str_double[i] == '0' && i > 1 && str_double[i - 1] != '.') {
    str_double[i] = '\0';
    i--;
  }
}

void append_calcule_array(char *string, json_msg *json) {
  double *arr = json->valeurs.double_values->num_array;
  char *ope = json->valeurs.double_values->operateur;
  char tmpstr[50], tmp2str[53];
  if (ope != NULL) {
    sprintf(tmpstr, " \"%s\",", ope);
    strcat(string, tmpstr);
  }
  size_t i = 0;
  for (i = 0; i < json->size - 1; i++) {
    memset(tmpstr, 0, sizeof(tmpstr));
    memset(tmp2str, 0, sizeof(tmp2str));
    sprintf(tmpstr, "%f", arr[i]);
    remove_zeros(tmpstr);
    sprintf(tmp2str, " %s,", tmpstr);
    strcat(string, tmp2str);
  }
  memset(tmpstr, 0, sizeof(tmpstr));
  memset(tmp2str, 0, sizeof(tmp2str));
  sprintf(tmpstr, "%f", arr[i]);
  remove_zeros(tmpstr);
  sprintf(tmp2str, " %s ", tmpstr);
  strcat(string, tmp2str);
}

// Met en forme au format json
int json_to_string(char *string, json_msg *json) {
  sprintf(string, "{\n\t\"code\" : \"%s\",\n\t\"valeurs\" : [", json->code);
  printf("%s\n", string);
  if (strcmp(json->code, "calcule") == 0) {
    append_calcule_array(string, json);
  } else {
    char tmpstr[2048];
    char **arr = json->valeurs.str_array;
    if (json->size > 0 && strcmp(*arr, "enregistré") &&
        (strcmp(json->code, "couleurs") == 0 ||
         strcmp(json->code, "balises") == 0)) {
      memset(tmpstr, 0, sizeof(tmpstr));
      sprintf(tmpstr, " %d,", json->size);
      strcat(string, tmpstr);
    }

    unsigned int i;
    for (i = 0; i < json->size - 1; i++) {
      memset(tmpstr, 0, sizeof(tmpstr));
      sprintf(tmpstr, " \"%s\",", arr[i]);
      strcat(string, tmpstr);
    }
    memset(tmpstr, 0, sizeof(tmpstr));
    sprintf(tmpstr, " \"%s\" ", arr[i]);
    strcat(string, tmpstr);
  }
  strcat(string, "]\n}");
  printf("%s\n", string);
  return 0;
}

void delete_s(void *ptr, char *name) {
  ptr != NULL ? free(ptr) : printf("%s is NULL, cannot free\n", name);
}

void delete_json(json_msg *json) {
  if (json != NULL) {
    if (strcmp(json->code, "calcule") == 0) {
      if (json->valeurs.double_values != NULL) {
        delete_s(json->valeurs.double_values->num_array, "num_array");
        delete_s(json->valeurs.double_values->operateur, "operateur");
        delete_s(json->valeurs.double_values, "calcule struct");
      }
    } else {
      if (json->valeurs.str_array != NULL) {
        for (uint i = 0; i < json->size; i++) {
          delete_s(json->valeurs.str_array[i], "string_array value");
        }
        delete_s(json->valeurs.str_array, "string_array");
      }
    }
  }
  delete_s(json, "json");
}