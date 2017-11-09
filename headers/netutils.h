#include "utils.h"
#include <stdio.h>
#include <sys/socket.h>
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
  MAX_SEG_SIZE = 512,
  INITIAL_WRITE_FLAG = 0,
  CHUNK_WRITE_FLAG = 1,
  FINAL_WRITE_FLAG = 2,
  INT_SIZE = 4,
  RESET_SIG = 'N',
  PROCEED_SIG = 'Y',
  CHUNK_INFO_STRUCT_SIZE = MAXCHARBUFF + NUM_SERVER * INT_SIZE
};

void fetch_and_print_error(int);
int encode_user_struct(char*, user_struct*);
void send_int_value_socket(int, int);
void send_signal(int*, int, u_char);
void recv_signal(int, u_char*);
void recv_int_value_socket(int, int*);
int send_to_socket(int, u_char*, int);
int recv_from_socket(int, u_char*, int);
void decode_user_struct(char*, user_struct*);
void encode_split_struct_to_buffer(u_char*, split_struct*);
void decode_split_struct_from_buffer(u_char*, split_struct*);
void encode_int_to_uchar(u_char*, int);
void decode_int_from_uchar(u_char*, int*);
void encode_server_chunks_info_struct_to_buffer(u_char*, server_chunks_info_struct*);
void decode_server_chunks_info_struct_from_buffer(u_char*, server_chunks_info_struct*);
void encode_chunk_info_struct_to_buffer(u_char*, chunk_info_struct*);
void decode_chunk_info_struct_from_buffer(u_char*, chunk_info_struct*);
void write_split_to_socket_as_stream(int, split_struct*);
void write_split_from_socket_as_stream(int, split_struct*);
#endif
