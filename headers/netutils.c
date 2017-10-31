#include "netutils.h"

int encode_user_struct(char* buffer, user_struct* user)
{
  int n_bytes;
  n_bytes = sprintf(buffer, AUTH_TEMPLATE, AUTH_FLAG, user->username, user->password);
  DEBUGSS("Auth String Sent", buffer);
  if (n_bytes < 0) {
    perror("Failed to Encode User Struct");
    exit(1);
  }
  return n_bytes;
}

void decode_user_struct(char* buffer, user_struct* user)
{
  int flag;
  DEBUGSS("Auth String Recv", buffer);
  if ((sscanf(buffer, AUTH_TEMPLATE, &flag, user->username, user->password) <= 0)) {
    perror("Failed to decode user struct string");
    exit(1);
  }
  DEBUGSS("Decoded Username", user->username);
  DEBUGSS("Decoded Password", user->password);
  if (flag != AUTH_FLAG) {
    fprintf(stderr, "Failed to decode User Struct");
    exit(1);
  }
}
