#include <assert.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "json.h"

char *OK_1 = "{\n\t\"code\" : \"nom\",\n\t\"valeurs\" : [ \"aze\" ]\n}";
char *OK_2 = "{\"code\":\"message\",\"valeurs\":[\"aze\"]}";
char *OK_3 = "{ \"code\"  :  \"couleurs\"  ,  \"valeurs\" :   [ 1,      "
             "\"#123456\"  ] }";
char *OK_4 = "{\"code\":\"balises\",\"valeurs\":[1, \"#balise\"]}";
char *OK_5 = "{\"code\":\"calcule\",\"valeurs\":[\"+\", 1, 5]}";

char *NOK_1 = "\"code\":\"nom\",\"valeurs\":[\"aze\"]}";
char *NOK_2 = "{code\":\"message\",\"valeurs\":[\"aze\"]}";
char *NOK_3 = "{\"code\":\"nom\",\"vars\":[\"aze\"]}";
char *NOK_4 = "{\"code\":\"nom\",\"valeurs\":\"aze\"]}";
char *NOK_5 = "{\"code\":\"mauvais code\",\"valeurs\":[\"aze\"]}";
char *NOK_6 = "{\"code\":\"nom\"\"valeurs\":[\"aze\"]}";

void test_validation_json() {
  json_msg **test_json = malloc(sizeof(json_msg *) * 11);
  for (size_t i = 0; i < 11; i++) {
    test_json[i] = (json_msg *)malloc(sizeof(json_msg));
  }
  assert(parse_json(OK_1, test_json[0]) == 0);
  assert(parse_json(OK_2, test_json[1]) == 0);
  assert(parse_json(OK_3, test_json[2]) == 0);
  assert(parse_json(OK_4, test_json[3]) == 0);
  assert(parse_json(OK_5, test_json[4]) == 0);

  assert(parse_json(NOK_1, test_json[5]) != 0);
  assert(parse_json(NOK_2, test_json[6]) != 0);
  assert(parse_json(NOK_3, test_json[7]) != 0);
  assert(parse_json(NOK_4, test_json[8]) != 0);
  assert(parse_json(NOK_5, test_json[9]) != 0);
  assert(parse_json(NOK_6, test_json[10]) != 0);
  for (size_t i = 0; i < 11; i++) {
    delete_json(test_json[i]);
  }
  free(test_json);
  printf("test validation json : Success\n");
}

char *send_receive_data(int socketfd, char *data) {
  static char result[1024];
  memset(result, 0, sizeof(result));
  int write_status = write(socketfd, data, strlen(data));
  if (write_status < 0) {
    perror("erreur ecriture");
    exit(EXIT_FAILURE);
  }
  char res[1024];
  memset(res, 0, sizeof(res));
  int read_status = read(socketfd, res, sizeof(res));
  if (read_status < 0) {
    perror("erreur lecture");
  }
  strcat(result, res);
  return result;
}

void test_nom(int socketfd) {
  char *nom = "{\"code\":\"nom\",\"valeurs\":[\"LeNom\"]}";
  assert(strcmp(nom, send_receive_data(socketfd, nom)) == 0);
}

void test_calcules(int socketfd) {
  char *c1 = "{\"code\":\"calcules\",\"valeurs\":[\"+\", 1, 2]}";
  char *res1 = trim(send_receive_data(socketfd, c1));
  char *exp1 = "{\"code\":\"calcules\",\"valeurs\":[3]}";
  assert(strcmp(res1, exp1) == 0);
}

void test_integrations() {
  int socketfd;
  struct sockaddr_in server_addr;
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8090);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  int connect_status =
      connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (connect_status < 0) {
    perror("connection serveur");
    exit(EXIT_FAILURE);
  }

  test_nom(socketfd);
  test_calcules(socketfd);
  /* test_couleurs(socketfd);
  test_balises(socketfd); */
}

// serveur pour un seul client
void *server(void *fct) {
  int socketfd, bind_status;
  socklen_t client_addr_len;
  struct sockaddr_in server_addr, client_addr;
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    perror("Unable to open a socket");
    pthread_exit(NULL);
  }
  int option = 1;
  setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8090);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bind_status =
      bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (bind_status < 0) {
    perror("bind");
    pthread_exit(NULL);
  }
  if (listen(socketfd, 10) < 0) {
    perror("listen");
    pthread_exit(NULL);
  }
  int client_socket_fd =
      accept(socketfd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_socket_fd < 0) {
    perror("accept");
    pthread_exit(NULL);
  }
  // fct(client_socket_fd);
  pthread_exit(NULL);
}

int main() {
  // create fake server
  /* pthread_t tid;
  memset(&tid, 0, sizeof(pthread_t));
  int i;
  pthread_create(&(tid), NULL, server, &i);
  pthread_detach(tid); */

  test_validation_json();
  // test_integrations();
  return 0;
}
