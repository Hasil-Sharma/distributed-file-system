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

void encode_int_to_uchar(u_char* buffer, int n)
{
  buffer[0] = (n >> 24) & 0xFF;
  buffer[1] = (n >> 16) & 0xFF;
  buffer[2] = (n >> 8) & 0xFF;
  buffer[3] = n & 0xFF;
}

void decode_int_from_uchar(u_char* buffer, int* n)
{
  int temp;

  temp = buffer[0] << 24;
  temp |= buffer[1] << 16;
  temp |= buffer[2] << 8;
  temp |= buffer[3];

  *n = temp;
}
void write_split_to_socket_as_stream(int socket, split_struct* split)
{
  // 1 byte for flag 4 bytes for split_id and 4 bytes for content_length

  u_char payload_buffer[MAX_SEG_SIZE];
  int bytes_sent = 0, content_bytes_sent, bytes_to_send_next;

  memset(payload_buffer, 0, sizeof(payload_buffer));
  payload_buffer[0] = INITIAL_WRITE_FLAG;
  encode_int_to_uchar(payload_buffer + 1, split->id);
  encode_int_to_uchar(payload_buffer + 5, split->content_length);
  bytes_to_send_next = split->content_length;

  bytes_to_send_next = (bytes_to_send_next < MAX_SEG_SIZE - 9) ? bytes_to_send_next : MAX_SEG_SIZE - 9;
  bcopy(split->content, payload_buffer + 9, bytes_to_send_next);

  while (bytes_sent != MAX_SEG_SIZE) {
    if ((bytes_sent += send(socket, payload_buffer + bytes_sent, MAX_SEG_SIZE - bytes_sent, 0)) < 0) {
      perror("Unable to send first split struct data");
    }
  }

  content_bytes_sent = bytes_to_send_next;
  DEBUGSN("content_bytes_sent", content_bytes_sent);
  if (split->content_length > MAX_SEG_SIZE - 9) {
    DEBUGS("Couldn't send entire buffer");

    while (content_bytes_sent != split->content_length) {
      if ((content_bytes_sent += send(socket, split->content + content_bytes_sent, split->content_length - content_bytes_sent, 0)) < 0) {
        perror("Unable to send split struct data");
      }
    }
  }
}

void write_split_from_socket_as_stream(int socket, split_struct* split)
{
  u_char payload_buffer[MAX_SEG_SIZE];
  int read_bytes = 0, content_bytes_recv, bytes_to_recv_next;

  memset(payload_buffer, 0, sizeof(payload_buffer));
  while (read_bytes != MAX_SEG_SIZE) {
    if ((read_bytes += recv(socket, payload_buffer + read_bytes, MAX_SEG_SIZE - read_bytes, 0)) < 0) {
      perror("Error in receiving write flag chunk");
    }
  }

  if (payload_buffer[0] == INITIAL_WRITE_FLAG) {
    DEBUGS("Initial Write Flag");

    decode_int_from_uchar(payload_buffer + 1, &split->id);
    decode_int_from_uchar(payload_buffer + 5, &split->content_length);

    DEBUGSN("Split ID", split->id);
    DEBUGSN("Content Length", split->content_length);

    split->content = (u_char*)malloc(split->content_length * sizeof(u_char));

    bytes_to_recv_next = split->content_length;
    bytes_to_recv_next = (bytes_to_recv_next < MAX_SEG_SIZE - 9) ? bytes_to_recv_next : MAX_SEG_SIZE - 9;
    bcopy(payload_buffer + 9, split->content, bytes_to_recv_next);
    content_bytes_recv = bytes_to_recv_next;
    DEBUGSN("content_bytes_recv", content_bytes_recv);
  }

  if (split->content_length > content_bytes_recv) {

    DEBUGS("Couldn't recevie entire buffer");

    while (content_bytes_recv != split->content_length) {
      if ((content_bytes_recv += recv(socket, split->content + content_bytes_recv, split->content_length - content_bytes_recv, 0)) < 0) {
        perror("Error in receiving write flag chunk");
      }
    }
  }
}

void encode_split_struct_to_buffer(u_char* buffer, split_struct* split)
{
  int i;
  u_char* ptr;

  encode_int_to_uchar(buffer + 1, split->id);
  encode_int_to_uchar(buffer + 5, split->content_length);
  ptr = &buffer[9];
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
