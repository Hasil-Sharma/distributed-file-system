#include "dfcutils.h"

int get_dfc_socket(dfc_server_struct* server)
{

  int sockfd;
  struct sockaddr_in serv_addr;
  struct timeval tv;
  tv.tv_sec = 1;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Unable to start socket:");
    exit(1);
  }

  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server->port);

  if (inet_pton(AF_INET, server->address, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    return -1;
  }

  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection Failed");
    return -1;
  }
  DEBUGSS("Connected to Server", server->name);
  return sockfd;
}

bool dfc_command_validator(char* buffer, int flag, file_attr_struct* file_attr)
{
  char *temp, temp_buffer[MAXCHARBUFF];
  int buffer_len, char_count, i;
  buffer_len = strlen(buffer);
  char_count = get_count_str_chr(buffer, ' ');
  memset(temp_buffer, 0, sizeof(temp_buffer));
  // Buffer is pointer to the first character of the command arguments
  if (flag == LIST_FLAG) {

    if (buffer_len == 0) {
      buffer[0] = ROOT_FOLDER_CHR;
      ++buffer_len;
    }
    extract_file_name_and_folder(buffer, file_attr, EXTRACT_REMOTE);

  } else if (flag == GET_FLAG || flag == PUT_FLAG) {

    if (char_count != 1) {
      if (flag == GET_FLAG)
        fprintf(stderr, "GET command not valid\n");
      else
        fprintf(stderr, "PUT command not valid\n");
      return false;
    }

    // temp now has local path and file_name together
    for (i = 0; i < 2; i++) {
      temp = get_token(buffer, SPACE_STR, i);
      memset(temp_buffer, 0, sizeof(temp_buffer));
      strcpy(temp_buffer, temp);
      free(temp);
      extract_file_name_and_folder(temp_buffer, file_attr,
          (i == 0) ? EXTRACT_LOCAL : EXTRACT_REMOTE);
    }
  } else if (flag == MKDIR_FLAG) {
    if (char_count != 0) {
      fprintf(stderr, "MKDIR command not valid\n");
      return false;
    }
    extract_file_name_and_folder(buffer, file_attr, EXTRACT_REMOTE);
  } else {
    fprintf(stderr, "<<< Unknown Command\n");
  }
  return true;
}

void dfc_command_builder(char* buffer, const char* format, file_attr_struct* file_attr, user_struct* user, int flag)
{
  char *file_folder, *file_name;
  file_folder = file_attr->remote_file_folder;
  file_name = file_attr->remote_file_name;

  file_folder = (strlen(file_folder) > 0) ? file_folder : "NULL";
  file_name = (strlen(file_name) > 0) ? file_name : "NULL";
  sprintf(buffer, format,
      flag,
      user->username,
      user->password,
      file_folder,
      file_name);

  DEBUGSS("Command Built", buffer);
}

void dfc_command_handler(int* conn_fds, int flag, char* buffer, dfc_conf_struct* conf)
{
  file_attr_struct file_attr;
  memset(&file_attr, 0, sizeof(file_attr));
  char buffer_to_send[MAX_SEG_SIZE];
  memset(buffer_to_send, 0, sizeof(buffer_to_send));
  if (dfc_command_validator(buffer, flag, &file_attr)) {

    if (flag == LIST_FLAG) {

      DEBUGS("LIST Validation Done");
      dfc_command_builder(buffer_to_send, LIST_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == GET_FLAG) {

      DEBUGS("GET Validation Done");
      if (strlen(file_attr.remote_file_name) == 0) {
        strcpy(file_attr.remote_file_name, file_attr.local_file_name);
      }
      dfc_command_builder(buffer_to_send, GET_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == PUT_FLAG) {

      DEBUGS("PUT Validation Done");
      if (strlen(file_attr.remote_file_name) == 0) {
        strcpy(file_attr.remote_file_name, file_attr.local_file_name);
      }
      dfc_command_builder(buffer_to_send, PUT_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == MKDIR_FLAG) {

      DEBUGS("MKDIR Validation Done");
      dfc_command_builder(buffer_to_send, MKDIR_TEMPLATE, &file_attr, conf->user, flag);
    }

    dfc_command_exec(conn_fds, buffer_to_send, conf->server_count, flag);
  } else {
    fprintf(stderr, "Failed to validate Command");
  }
}

bool send_command(int* conn_fds, char* buffer_to_send, int conn_count)
{
  int i, buffer_len, s_bytes;
  buffer_len = strlen(buffer_to_send);

  for (i = 0; i < conn_count; i++) {
    if (conn_fds[i] == -1)
      continue;
    if ((s_bytes = send(conn_fds[i], buffer_to_send, MAXCHARBUFF, 0)) < 0) {
      return false;
    }
    if (s_bytes == 0)
      conn_fds[i] = -1;
  }

  return true;
}
void dfc_command_exec(int* conn_fds, char* buffer_to_send, int conn_count, int flag)
{
  int i;
  bool send_flag;
  send_flag = send_command(conn_fds, buffer_to_send, conn_count);

  if (flag == LIST_FLAG) {

    if (!send_flag)
      perror("Unable to send LIST");
    DEBUGS("Sent LIST");
  } else if (flag == GET_FLAG) {

    if (!send_flag)
      perror("Unable to send GET");

    DEBUGS("Sent GET");
  } else if (flag == PUT_FLAG) {

    if (!send_flag)
      perror("Unable to send PUT");

    DEBUGS("Sent PUT");
  } else if (flag == MKDIR_FLAG) {

    if (!send_flag)
      perror("Unable to send MKDIR");

    DEBUGS("Sent MKDIR");
  }
}
void create_dfc_to_dfs_connections(int* conn_fds, dfc_conf_struct* conf)
{
  int i;
  for (i = 0; i < conf->server_count; i++) {
    conn_fds[i] = get_dfc_socket(conf->servers[i]);
    if (conn_fds[i] == -1)
      DEBUGSS("Couldn't Connect to Server", conf->servers[i]->name);
  }
}

bool auth_dfc_to_dfs_connections(int* conn_fds, dfc_conf_struct* conf)
{
  int i, n_bytes, socket, s_bytes, r_bytes;
  bool ans = false;
  char buffer[MAX_SEG_SIZE], response[MAX_SEG_SIZE];
  memset(buffer, 0, sizeof(buffer));
  n_bytes = encode_user_struct(buffer, conf->user);

  for (i = 0; i < conf->server_count; i++) {
    socket = conn_fds[i];

    if (socket != -1) {

      ans = true;
      if ((s_bytes = send(socket, buffer, n_bytes, 0)) != n_bytes) {
        fprintf(stderr, "Failed to send auth message\n");
        return false;
      }

      memset(response, 0, sizeof(response));
      if ((r_bytes = recv(socket, response, MAX_SEG_SIZE, 0)) <= 0) {
        fprintf(stderr, "Failed to recv auth message\n");
        return false;
      }

      if (strcmp(response, AUTH_OK) != 0)
        return false;
    }
  }

  return ans;
}

void setup_dfc_to_dfs_connections(int** conn_fds, dfc_conf_struct* conf)
{
  (*conn_fds) = (int*)malloc(conf->server_count * sizeof(int));
  create_dfc_to_dfs_connections(*conn_fds, conf);
}

void read_dfc_conf(char* file_path, dfc_conf_struct* conf)
{
  FILE* fp;
  char line[MAXFILEBUFF];
  int line_len;
  if ((fp = fopen(file_path, "r")) <= 0) {
    perror("DFC => Error in opening config file: ");
    exit(1);
  }

  while (fgets(line, sizeof(line), fp)) {
    // Assumption that entire line can fix in this buffer
    line_len = strlen(line);
    line[line_len - 1] = (line[line_len - 1] == NEW_LINE_CHAR) ? NULL_CHAR : line[line_len - 1];
    if (get_sub_string(line, DFC_SERVER_CONF)) {
      insert_dfc_server_conf(line, conf);
    } else if (get_sub_string(line, DFC_USERNAME_CONF)) {
      insert_dfc_user_conf(line, conf, DFC_USERNAME_DELIM, USERNAME_FLAG);
    } else if (get_sub_string(line, DFC_PASSWORD_CONF)) {
      insert_dfc_user_conf(line, conf, DFC_PASSWORD_DELIM, PASSWORD_FLAG);
    }
  }
}

bool check_dfc_server_struct(dfc_server_struct** server)
{
  if (*server == NULL) {
    *server = (dfc_server_struct*)malloc(sizeof(dfc_server_struct));
    return false;
  }
  return true;
}

void insert_dfc_server_conf(char* line, dfc_conf_struct* conf)
{
  char *ptr1, *ptr2;
  int len, i;

  ptr1 = get_sub_string_after(line, SPACE_STR);
  ptr2 = get_sub_string_after(ptr1, SPACE_STR);
  len = ptr2 - ptr1 - 1;

  i = conf->server_count++;
  assert(check_dfc_server_struct(&conf->servers[i]) == false);

  conf->servers[i]->name = strndup(ptr1, len);
  ptr1 = get_sub_string_after(ptr2, COLON_STR);

  len = ptr1 - ptr2 - 1;
  conf->servers[i]->address = strndup(ptr2, len);
  conf->servers[i]->port = atoi(ptr1);
}

void insert_dfc_user_conf(char* line, dfc_conf_struct* conf, char* delim, int flag)
{
  char* ptr;
  check_user_struct(&conf->user);
  ptr = get_sub_string_after(line, delim);
  if (flag == PASSWORD_FLAG)
    conf->user->password = strdup(ptr);
  else
    conf->user->username = strdup(ptr);
}

bool split_file_to_pieces(char* file_path, file_split_struct* file_split, int n)
{
  FILE* fp;
  int i;
  long long file_size, split_size, rem_size;
  struct stat sb;
  split_struct* split;

  if ((fp = fopen(file_path, "rb")) <= 0) {
    fprintf(stderr, "Unable to open file: %s\n", file_path);
    return false;
  }

  if (stat(file_path, &sb) == -1) {
    fprintf(stderr, "Error in getting file size: %s\n", file_path);
    return false;
  }

  file_split->file_name = strdup(file_path);

  file_size = (long long)sb.st_size;
  DEBUGSN("File Size read", (int)file_size);

  split_size = file_size / n; // First n - 1 splits will have split_size number of bytes
  rem_size = file_size % n;   // Last nth split will have split_size + rem_size number of bytes

  for (i = 0; i < n; i++) {
    file_split->splits[i] = (split_struct*)malloc(sizeof(split_struct));
    split = file_split->splits[i];
    split->id = i + 1;
    split->content_length = (i != n - 1) ? split_size : split_size + rem_size;
    split->content = (u_char*)malloc(split->content_length * sizeof(u_char));
    fread(split->content, sizeof(u_char), split->content_length, fp);
    file_split->split_count++;
  }

  fclose(fp);

  return true;
}

bool combine_file_from_pieces(char* file_path, file_split_struct* file_split)
{
  FILE* fp;
  int i;
  split_struct* split;
  char file_name[MAXFILEBUFF];

  memset(file_name, 0, sizeof(file_name));
  sprintf(file_name, "%s/%s", file_path, file_split->file_name);

  if ((fp = fopen(file_name, "wb")) <= 0) {
    fprintf(stderr, "Unable to open file: %s\n", file_path);
    return false;
  }

  for (i = 0; i < file_split->split_count; i++) {
    split = file_split->splits[i];
    fwrite(split->content, sizeof(u_char), split->content_length, fp);
  }
  fclose(fp);
}

void print_dfc_conf_struct(dfc_conf_struct* conf)
{
  int i;
  dfc_server_struct* ptr;
  DEBUGSS("Username", conf->user->username);
  DEBUGSS("Password", conf->user->password);
  for (i = 0; i < conf->server_count; i++) {
    ptr = conf->servers[i];
    fprintf(stderr, "DEBUG: Name:%s Address:%s Port:%d\n", ptr->name, ptr->address, ptr->port);
  }
}

void free_dfc_conf_struct(dfc_conf_struct* conf)
{
  int i;
  free_user_struct(conf->user);
  i = conf->server_count;
  while (i--) {
    free_dfc_server_struct(conf->servers[i]);
  }
}

void free_dfc_server_struct(dfc_server_struct* server)
{
  free(server->name);
  free(server->address);
}
