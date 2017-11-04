#include "dfsutils.h"

int get_dfs_socket(int port_number)
{

  int sockfd;
  struct sockaddr_in sin;
  int yes = 1;

  memset(&sin, 0, sizeof(sin));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(port_number);
  sin.sin_addr.s_addr = INADDR_ANY;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Unable to start socket:");
    exit(1);
  }

  // Avoiding the "Address Already in use" error message
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    perror("Unable to set so_reuseaddr:");
    exit(1);
  }

  if (bind(sockfd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
    perror("Unable to bind the socket:");
    exit(1);
  }

  if (listen(sockfd, MAXCONNECTION) < 0) {
    perror("Unable to call listen on socket:");
    exit(1);
  }

  return sockfd;
}
bool dfs_command_decode_and_auth(char* buffer, const char* format, dfs_recv_command_struct* recv_cmd, dfs_conf_struct* conf)
{
  int flag;
  user_struct* user;
  user = &recv_cmd->user;
  sscanf(buffer, LIST_TEMPLATE,
      &flag,
      user->username,
      user->password,
      recv_cmd->folder,
      recv_cmd->file_name);

  DEBUGSS("Username", user->username);
  DEBUGSS("Password", user->password);
  DEBUGSS("Folder", recv_cmd->folder);
  DEBUGSS("Filename", recv_cmd->file_name);

  return auth_dfs_user(user, conf);
}
void dfs_command_accept(int socket, dfs_conf_struct* conf)
{
  char buffer[MAX_SEG_SIZE], temp_buffer[MAX_SEG_SIZE], c;
  int r_bytes, flag;
  user_struct* user;
  dfs_recv_command_struct dfs_recv_command;

  memset(buffer, 0, sizeof(buffer));
  memset(temp_buffer, 0, sizeof(temp_buffer));
  memset(&dfs_recv_command, 0, sizeof(dfs_recv_command));

  dfs_recv_command.user.username = (char*)malloc(MAXCHARBUFF * sizeof(char));
  dfs_recv_command.user.password = (char*)malloc(MAXCHARBUFF * sizeof(char));

  user = &dfs_recv_command.user;

  // receiving the command
  DEBUGS("Recv the command");
  recv_from_socket(socket, buffer, MAX_SEG_SIZE);

  c = 'Y';
  // This to let client know about the non-existent servers
  DEBUGS("Sending the Alive ACK");
  send_to_socket(socket, &c, 1);

  DEBUGSS("Command Received", buffer);
  sscanf(buffer, GENERIC_TEMPATE, &dfs_recv_command.flag, temp_buffer);
  /*DEBUGSN("Flag", dfs_recv_command.flag);*/
  flag = dfs_recv_command.flag;

  if (flag == LIST_FLAG) {

    DEBUGS("Command Received is LIST");
    if (!dfs_command_decode_and_auth(buffer, LIST_TEMPLATE, &dfs_recv_command, conf)) {
      DEBUGSS("Failed to authenticate", user->username);
    }

    DEBUGSS("Authenticated", user->username);

  } else if (flag == GET_FLAG) {

    DEBUGS("Command Received is GET");
    if (!dfs_command_decode_and_auth(buffer, GET_TEMPLATE, &dfs_recv_command, conf)) {
      DEBUGSS("Failed to authenticate", user->username);
    }

    DEBUGSS("Authenticated", user->username);
  } else if (flag == PUT_FLAG) {

    DEBUGS("Command Received is PUT");
    if (!dfs_command_decode_and_auth(buffer, PUT_TEMPLATE, &dfs_recv_command, conf)) {
      DEBUGSS("Failed to authenticate", user->username);
    }

    DEBUGSS("Authenticated", user->username);
  } else if (flag == MKDIR_FLAG) {

    DEBUGS("Command Received is MKDIR");
    if (!dfs_command_decode_and_auth(buffer, MKDIR_TEMPLATE, &dfs_recv_command, conf)) {
      DEBUGSS("Failed to authenticate", user->username);
    }

    DEBUGSS("Authenticated", user->username);
  }

  dfs_command_exec(socket, &dfs_recv_command, conf, flag);

  free(dfs_recv_command.user.username);
  free(dfs_recv_command.user.password);
}

bool dfs_command_exec(int socket, dfs_recv_command_struct* recv_cmd, dfs_conf_struct* conf, int flag)
{
  char char_buffer[2 * MAXCHARBUFF];
  u_char payload_buffer[MAX_SEG_SIZE], *u_char_buffer;
  server_chunks_info_struct server_chunks_info;
  split_struct splits[2];
  int len, i, r_bytes, size_of_payload;

  memset(char_buffer, 0, sizeof(char_buffer));
  memset(&splits, 0, sizeof(splits));
  memset(&server_chunks_info, 0, sizeof(server_chunks_info_struct));
  len = sprintf(char_buffer, "%s/%s/%s", conf->server_name, recv_cmd->user.username, recv_cmd->folder);

  // Handling case when recv_cmd->folder is '/'
  if (char_buffer[len - 1] == ROOT_FOLDER_CHR && char_buffer[len - 2] == ROOT_FOLDER_CHR)
    char_buffer[--len] = NULL_CHAR;

  DEBUGSS("Generated folder path", char_buffer);

  if (flag == LIST_FLAG) {
    get_files_in_folder(char_buffer, &server_chunks_info);
    print_server_chunks_info_struct(&server_chunks_info);

    size_of_payload = INT_SIZE + server_chunks_info.chunks * CHUNK_INFO_STRUCT_SIZE;

    // First send the size_of_payload

    send_int_value_socket(socket, size_of_payload);
    u_char_buffer = (u_char*)malloc(sizeof(u_char) * size_of_payload);
    encode_server_chunks_info_struct_to_buffer(u_char_buffer, &server_chunks_info);
    send_to_socket(socket, u_char_buffer, size_of_payload);
    free(u_char_buffer);
  } else if (flag == GET_FLAG) {
  } else if (flag == PUT_FLAG) {

    create_dfs_directory(char_buffer);
    for (i = 0; i < 2; i++) {

      memset(payload_buffer, 0, sizeof(payload_buffer));
      /*r_bytes = 0;*/
      /*while (r_bytes != MAX_SEG_SIZE) {*/
      /*if ((r_bytes += recv(socket, payload_buffer, MAX_SEG_SIZE, 0)) <= 0)*/
      /*perror("Failed to receive file_split");*/
      /*DEBUGSN("Bytes Received: dfs_command_exec", r_bytes);*/
      /*}*/
      write_split_from_socket_as_stream(socket, &splits[i]);
      DEBUGS("Split struct read complete");

      DEBUGS("Md5 hash Sum");
      print_hash_value(splits[i].content, splits[i].content_length);
      /*decode_split_struct_from_buffer(payload_buffer, &splits[i]);*/
      /*DEBUGSS("Payload Buffer", (char*)payload_buffer);*/
      /*DEBUGSN("Received file_split id", splits[i].id);*/
      write_split_struct_to_file(&splits[i], char_buffer, recv_cmd->file_name);
    }
  } else if (flag == MKDIR_FLAG) {
    create_dfs_directory(char_buffer);
  }
}

bool auth_dfs_user(user_struct* user, dfs_conf_struct* conf)
{
  int i;
  for (i = 0; i < conf->user_count; i++) {
    if (compare_user_struct(user, conf->users[i])) {
      DEBUGSS("auth_dfs_user: Authenticated", user->username);
      return true;
    }
  }
  DEBUGSS("Couldn't Authenticate", user->username);
  return false;
}

void read_dfs_conf(char* file_path, dfs_conf_struct* conf)
{
  FILE* fp;
  char line[MAXFILEBUFF];
  int line_len;
  if ((fp = fopen(file_path, "r")) <= 0) {
    perror("DFC => Error in opening config file: ");
  }

  while (fgets(line, sizeof(line), fp)) {
    // Assumption that entire line can fix in this buffer
    line_len = strlen(line);
    line[line_len - 1] = (line[line_len - 1] == NEW_LINE_CHAR) ? NULL_CHAR : line[line_len - 1];
    insert_dfs_user_conf(line, conf);
  }
}

void insert_dfs_user_conf(char* line, dfs_conf_struct* conf)
{
  char* ptr;
  int len, i;
  ptr = get_sub_string_after(line, SPACE_STR);
  len = ptr - line - 1;

  i = conf->user_count++;
  assert(check_user_struct(&conf->users[i]) == false);
  conf->users[i]->username = strndup(line, len);
  conf->users[i]->password = strdup(ptr);
}

void create_dfs_directory(char* path)
{
  struct stat st;
  char *folder, *temp;
  int len;
  temp = path;
  len = 0;
  while (true) {
    memset(&st, 0, sizeof(st));
    folder = get_sub_string_after(temp, ROOT_FOLDER_STR);
    if (!folder)
      break;
    len += folder - temp;
    temp = folder;
    folder = strndup(path, len);
    DEBUGSS("Parsed Folder", folder);
    if (stat(folder, &st) == -1)
      mkdir(folder, 0755);
    free(folder);
  }
}

// Creates directory for all the servers with sub directory for each of the user
void dfs_directory_creator(char* server_name, dfs_conf_struct* conf)
{
  int i;
  char file_path[MAXFILEBUFF];
  user_struct* user;
  create_dfs_directory(server_name);
  for (i = 0; i < conf->user_count; i++) {
    user = conf->users[i];
    memset(file_path, 0, sizeof(file_path));
    sprintf(file_path, "%s/%s", server_name, user->username);
    create_dfs_directory(file_path);
  }
}

void print_dfs_conf_struct(dfs_conf_struct* conf)
{
  int i;
  user_struct* ptr;
  for (i = 0; i < conf->user_count; i++) {
    ptr = conf->users[i];
    fprintf(stderr, "Username: %s & Password: %s\n", ptr->username, ptr->password);
  }
}

void free_dfs_conf_struct(dfs_conf_struct* conf)
{
  int i;
  i = conf->user_count;
  while (i--)
    free_user_struct(conf->users[i]);
}
