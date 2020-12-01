#include "json.h"

char *OK_1 = "{\n\t\"code\" : \"nom\",\n\t\"valeurs\" : [ \"aze\" ]\n}";
char *OK_2 = "{\"code\":\"nom\",\"valeurs\":[\"aze\"]}";
char *OK_3 = "{ \"code\"  :  \"nom\"  ,  \"valeurs\" :   [      \"aze\"  ] }";

char *NOK_1 = "\"code\":\"nom\",\"valeurs\":[\"aze\"]}";
char *NOK_2 = "{code\":\"nom\",\"valeurs\":[\"aze\"]}";
char *NOK_3 = "{\"code\"\"nom\",\"valeurs\":[\"aze\"]}";
char *NOK_4 = "{\"code\":\"nom\",\"valeurs\":\"aze\"]}";
char *NOK_5 = "{\"code\":\"nom\",\"valeurs\":[\"aze\"]}";
char *NOK_6 = "{\"code\":\"nom\",\"valeurs\":[\"aze\"]}";

int main() {}