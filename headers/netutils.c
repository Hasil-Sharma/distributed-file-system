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

void encode_split_struct_to_buffer(u_char* buffer, split_struct* split)
{
  int i;
  u_char* ptr;
  buffer[0] = (u_char)split->id;
  buffer[1] = (u_char)split->content_length;
  ptr = &buffer[2];
  for (i = 0; i < split->content_length; i++)
    ptr[i] = split->content[i];
  DEBUGS("Encoding split_struct Done");
}

void decode_split_struct_from_buffer(u_char* buffer, split_struct* split)
{
  int i;
  u_char* ptr;
  fprintf(stderr, "First Byte %u\n", buffer[0]);
  if (buffer[0] == 0) {
    buffer = &buffer[1];
    DEBUGS("Advancing Pointer");
  }
  split->id = (int)buffer[0];
  split->content_length = (int)buffer[1];
  split->content = (u_char*)malloc(split->content_length * sizeof(u_char));
  ptr = &buffer[2];
  for (i = 0; i < split->content_length; i++)
    split->content[i] = ptr[i];
  DEBUGS("Decoding split_struct Done");
}
