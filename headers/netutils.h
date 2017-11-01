#include "utils.h"
#include <stdio.h>
#ifndef NETUTILS_H
#define NETUTILS_H

#define GENERIC_TEMPATE "FLAG %d %[^\n]s"
#define AUTH_TEMPLATE "FLAG %d USERNAME %s PASSWORD %s"
#define GET_TEMPLATE AUTH_TEMPLATE " FOLDER %s FILENAME %s\n"
#define PUT_TEMPLATE GET_TEMPLATE
#define LIST_TEMPLATE AUTH_TEMPLATE " FOLDER %s FILENAME %s\n"
#define MKDIR_TEMPLATE LIST_TEMPLATE
#define AUTH_OK "AUTH_OK"
#define AUTH_NOT_OK "AUTH_NOT_OK"

enum CommonConstants {
  LIST_FLAG = 0,
  GET_FLAG = 1,
  PUT_FLAG = 2,
  MKDIR_FLAG = 3,
  AUTH_FLAG = 4,
  MAX_SEG_SIZE = 512
};

int encode_user_struct(char*, user_struct*);
void decode_user_struct(char*, user_struct*);
void encode_split_struct_to_buffer(u_char*, split_struct*);
void decode_split_struct_from_buffer(u_char*, split_struct*);
#endif
