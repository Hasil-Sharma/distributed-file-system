#include "dfcutils.h"

int file_pieces_mapping[4][4][2] = {
  { { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 1 } },
  { { 4, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 } },
  { { 3, 4 }, { 4, 1 }, { 1, 2 }, { 2, 3 } },
  { { 2, 3 }, { 3, 4 }, { 4, 1 }, { 1, 2 } }
};

void setup_dfc_to_dfs_connections(int** conn_fds, dfc_conf_struct* conf)
{
  /* Setup dfc to dfs TCP Connections, after allocating the number of servers connections
   */
  (*conn_fds) = (int*)malloc(conf->server_count * sizeof(int));
  create_dfc_to_dfs_connections(*conn_fds, conf);
}

void tear_dfc_to_dfs_connections(int* conn_fds, dfc_conf_struct* conf)

{
  int i;
  for (i = 0; i < conf->server_count; i++) {
    if (conn_fds[i] == -1)
      continue;
    close(conn_fds[i]);
    /*DEBUGSN("Disconnected from Server", i + 1);*/
  }
}
void create_dfc_to_dfs_connections(int* conn_fds, dfc_conf_struct* conf)
{
  /* Creating a socket connection and adding timeout of 1 sec on recv
   */
  int i;
  for (i = 0; i < conf->server_count; i++) {
    conn_fds[i] = get_dfc_socket(conf->servers[i]);
    if (conn_fds[i] == -1) {
      /*DEBUGSS("Couldn't Connect to Server", conf->servers[i]->name);*/
      continue;
    }
    /*DEBUGSS("Connected to Server", conf->servers[i]->name);*/
  }
}

int get_dfc_socket(dfc_server_struct* server)
{
  /* Create a client socket with timeout of 1 sec on recv
   */
  int sockfd;
  struct sockaddr_in serv_addr;
  struct timeval tv;
  memset(&tv, 0, sizeof(tv));
  tv.tv_sec = 1;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Unable to start socket:");
    exit(1);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) < 0) {
    perror("Unable to set timeout");
    exit(1);
  }

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server->port);

  if (inet_pton(AF_INET, server->address, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    exit(1);
  }

  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection Failed");
    return -1;
  }
  //DEBUGSS("Connected to Server", server->name);
  return sockfd;
}

bool dfc_command_builder(char* buffer, const char* format, file_attr_struct* file_attr, user_struct* user, int flag)
{
  char *file_folder, *file_name;

  file_folder = file_attr->remote_file_folder;
  file_name = file_attr->remote_file_name;

  if (flag == LIST_FLAG) {

    file_folder = (strlen(file_folder) > 0) ? file_folder : ROOT_FOLDER_STR;
    file_name = (strlen(file_name) > 0) ? file_name : "NULL";

  } else if (flag == GET_FLAG) {

    file_folder = (strlen(file_folder) > 0) ? file_folder : ROOT_FOLDER_STR;

    // File name in GET cannot be null
    if (strlen(file_name) == 0)
      return false;

    if (strlen(file_attr->local_file_folder) > 1 && !check_directory_exists(file_attr->local_file_folder)) {
      printf("<<< local directory doesn't exist: %s\n", file_attr->local_file_folder);
      return false;
    }

  } else if (flag == PUT_FLAG) {

    file_folder = (strlen(file_folder) > 0) ? file_folder : ROOT_FOLDER_STR;
    // File name in PUT cannot be null
    if (strlen(file_name) == 0)
      return false;

    if (strlen(file_attr->local_file_folder) > 1 && !check_directory_exists(file_attr->local_file_folder)) {
      printf("<<< local directory doesn't exist: %s\n", file_attr->local_file_folder);
      return false;
    }

    if (!check_file_exists(file_attr->local_file_folder, file_attr->local_file_name)) {
      printf("<<< local file doesn't exist: %s%s\n", file_attr->local_file_folder, file_attr->local_file_name);
      return false;
    }
  } else if (flag == MKDIR_FLAG) {

    // File folder name cannot be null
    if (strlen(file_folder) == 0)
      return false;

    // There should be no file_name but sending one for easy of parsing later

    if (strlen(file_name) > 0)
      return false;
    file_name = "NULL";
  }

  sprintf(buffer, format,
      flag,
      user->username,
      user->password,
      file_folder,
      file_name);

  DEBUGSS("Command Built", buffer);
  return true;
}

void dfc_command_handler(int* conn_fds, int flag, char* buffer, dfc_conf_struct* conf)
{
  /* handles command on basis of the flag
   */

  file_attr_struct file_attr;
  char buffer_to_send[MAX_SEG_SIZE];
  bool builder_flag;
  memset(buffer_to_send, 0, sizeof(buffer_to_send));
  memset(&file_attr, 0, sizeof(file_attr));

  DEBUGS("Validating the command input");
  if (dfc_command_validator(buffer, flag, &file_attr)) {

    DEBUGS("Building the command to be send");
    if (flag == LIST_FLAG) {

      builder_flag = dfc_command_builder(buffer_to_send, LIST_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == GET_FLAG) {

      if (strlen(file_attr.remote_file_name) == 0) {
        strcpy(file_attr.remote_file_name, file_attr.local_file_name);
      }
      builder_flag = dfc_command_builder(buffer_to_send, GET_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == PUT_FLAG) {

      if (strlen(file_attr.remote_file_name) == 0) {
        strcpy(file_attr.remote_file_name, file_attr.local_file_name);
      }
      builder_flag = dfc_command_builder(buffer_to_send, PUT_TEMPLATE, &file_attr, conf->user, flag);

    } else if (flag == MKDIR_FLAG) {

      builder_flag = dfc_command_builder(buffer_to_send, MKDIR_TEMPLATE, &file_attr, conf->user, flag);
    }

    if (!builder_flag) {
    } else {
      DEBUGS("Creating Connections");
      create_dfc_to_dfs_connections(conn_fds, conf);
      DEBUGS("Executing the command on remote servers");
      dfc_command_exec(conn_fds, buffer_to_send, conf->server_count, &file_attr, flag, conf);
      DEBUGS("Tearing down connections");
      tear_dfc_to_dfs_connections(conn_fds, conf);
    }
  } else {
    fprintf(stderr, "Failed to validate Command\n");
  }
}

bool dfc_command_validator(char* buffer, int flag, file_attr_struct* file_attr)
{
  /* Tests whether command and its arguments entered are valid or not
   * Buffer is what remains after removing the Command and consecutive space except in LIST
   */

  // Add condition for each command what should be NULL and what not
  char *temp, temp_buffer[MAXCHARBUFF];
  int buffer_len, char_count, i;
  buffer_len = strlen(buffer);

  // Counting number of spaces in the buffer
  char_count = get_count_str_chr(buffer, ' ');
  memset(temp_buffer, 0, sizeof(temp_buffer));
  // Buffer is pointer to the first character of the command arguments
  if (flag == LIST_FLAG) {

    // In case only LIST without a folder is specified or it has trailing space
    if (buffer_len == 0 || buffer_len == char_count) {
      buffer[0] = ROOT_FOLDER_CHR;
      buffer_len = 1;
    }

    // name and name/ are different first implies file named "name" and later is folder named "name"
    extract_file_name_and_folder(buffer, file_attr, EXTRACT_REMOTE);

  } else if (flag == GET_FLAG || flag == PUT_FLAG) {

    if (char_count != 1) {
      if (flag == GET_FLAG)
        fprintf(stderr, "GET command not valid\n");
      else
        fprintf(stderr, "PUT command not valid\n");
      return false;
    }

    for (i = 0; i < 2; i++) {

      temp = get_token(buffer, SPACE_STR, i);
      // temp now has folder/filename type data depending on value of i
      memset(temp_buffer, 0, sizeof(temp_buffer));
      strcpy(temp_buffer, temp);
      free(temp);

      // extracting file name and folder corresponding to remote and local folder/filename
      extract_file_name_and_folder(temp_buffer, file_attr, (i == 0) ? EXTRACT_LOCAL : EXTRACT_REMOTE);
    }
  } else if (flag == MKDIR_FLAG) {

    if (char_count != 0) {
      // MKDIR doesn't expect any arguements thus
      fprintf(stderr, "MKDIR command not valid\n");
      return false;
    }

    // if argument doesn't end with a '/' do it explicitly
    if (buffer[buffer_len - 1] != ROOT_FOLDER_CHR) {
      buffer[buffer_len] = ROOT_FOLDER_CHR;
      ++buffer_len;
    }

    extract_file_name_and_folder(buffer, file_attr, EXTRACT_REMOTE);
  } else {
    fprintf(stderr, "<<< Unknown Command\n");
  }
  return true;
}

bool send_command(int* conn_fds, char* buffer_to_send, int conn_count)
{
  int i, buffer_size;
  bool ret_val = false; // To make sure atleast one server is running
  buffer_size = strlen(buffer_to_send);
  for (i = 0; i < conn_count; i++) {
    if (conn_fds[i] == -1)
      continue;

    // Sending number of bytes in command followed by the command
    send_int_value_socket(conn_fds[i], buffer_size);
    send_to_socket(conn_fds[i], buffer_to_send, buffer_size);
    ret_val = true;
  }

  return ret_val;
}

void send_file_splits(int socket, file_split_struct* file_split, int mod, int server_idx)
{

  u_char payload_buffer[MAX_SEG_SIZE];
  int file_piece, i;
  split_struct* split;

  /*DEBUGSN("Server Idx", server_idx);*/
  for (i = 0; i < CHUNKS_PER_SERVER; i++) {
    memset(payload_buffer, 0, sizeof(payload_buffer));
    file_piece = file_pieces_mapping[mod][server_idx][i];
    /*DEBUGSN("Uploading to Server", server_idx + 1);*/
    /*DEBUGSN("Uploading piece", file_piece);*/
    split = file_split->splits[file_piece - 1];
    assert(split->id == file_piece);
    /*DEBUGS("Split Hash_Value");*/
    /*print_hash_value(split->content, split->content_length);*/
    /*DEBUGSN("Split Content Length", split->content_length);*/
    write_split_to_socket_as_stream(socket, split);
  }
}

int fetch_remote_file_info(int* conn_fds, int conn_count, server_chunks_collate_struct* server_chunks_collate)
{

  /* Fetches file info from all the servers that is chunks stored in all each of the server
   * Returning value of mod on the basis of chunks recevied.
   * TODO: In case file doesn't exist on remote server
   */
  int i, j, payload_size, mod = -1;
  u_char* payload;
  server_chunks_info_struct server_chunks_info;
  chunk_info_struct* chunk_info;

  for (i = 0; i < conn_count; i++) {
    if (conn_fds[i] == -1)
      continue;

    // Receive the size of packet form server
    recv_int_value_socket(conn_fds[i], &payload_size);

    /*if (payload_size == -1) {*/
    /*DEBUGS("Error in fetching remote file info");*/
    /*fetch_and_print_error(conn_fds[i]);*/
    /*}*/
    payload = (u_char*)malloc(sizeof(u_char) * payload_size);
    memset(payload, 0, sizeof(payload));

    // Recv the packet from server
    recv_from_socket(conn_fds[i], payload, payload_size);

    // Decoding the received buffer into struct
    memset(&server_chunks_info, 0, sizeof(server_chunks_info_struct));
    decode_server_chunks_info_struct_from_buffer(payload, &server_chunks_info);

    // Inserting decoded strct into collate struct
    insert_to_server_chunks_collate_struct(server_chunks_collate, &server_chunks_info);

    // Finding mod corresponding to the chunk received
    chunk_info = server_chunks_info.chunk_info;
    for (j = 0; mod < 0; j++) {
      if ((file_pieces_mapping[j][i][0] == chunk_info->chunks[0] && file_pieces_mapping[j][i][1] == chunk_info->chunks[1]) || (file_pieces_mapping[j][i][0] == chunk_info->chunks[1] && file_pieces_mapping[j][i][1] == chunk_info->chunks[0])) {
        mod = j;
      }
    }

    // Freeing up memory allocated earlier
    free(payload);
  }

  return mod;
}

void fetch_remote_splits(int* conn_fds, int conn_count, file_split_struct* file_split, int mod)
{

  // Fetching file_split from remote servers with using the previous server in case current server is not available
  int i, socket, split_id, server;
  bool flag;
  u_char proceed_sig = (u_char)PROCEED_SIG;

  //DEBUGS("Fetching Remote Split");
  for (i = 0; i < conn_count; i++) {
    flag = false;
    socket = conn_fds[i];
    server = i + 1;
    if (socket == -1) {
      //DEBUGSN("Server is down", i + 1);
      server = (i != 0) ? i : conn_count;
      socket = conn_fds[(i != 0) ? (i - 1) : conn_count - 1];
      flag = (i == 0) ? true : false;
      if (i != 0) {
        // Socket is expeciing a sig
        send_to_socket(socket, &proceed_sig, sizeof(u_char));
      }
    }
    split_id = file_pieces_mapping[mod][i][0];
    /*fprintf(stderr, "Fetching split: %d from Server: %d\n", split_id, server);*/
    send_int_value_socket(socket, split_id);

    file_split->split_count++;
    file_split->splits[split_id - 1] = (split_struct*)malloc(sizeof(split_struct));
    write_split_from_socket_as_stream(socket, file_split->splits[split_id - 1]);

    if (flag)
      send_to_socket(socket, &proceed_sig, sizeof(u_char));
  }

  send_signal(conn_fds, conn_count, (u_char)RESET_SIG);
}

void dfc_command_exec(int* conn_fds, char* buffer_to_send, int conn_count, file_attr_struct* attr, int flag, dfc_conf_struct* conf)
{
  bool send_flag, error_flag;
  char file_path[MAXCHARBUFF];
  int mod, i, c;
  file_split_struct file_split;
  server_chunks_collate_struct server_chunks_collate;
  memset(file_path, 0, sizeof(file_path));
  memset(&server_chunks_collate, 0, sizeof(server_chunks_collate));
  memset(&file_split, 0, sizeof(file_split_struct));

  // Sending Command to Servers

  DEBUGS("Sending the command over to the servers");

  send_flag = send_command(conn_fds, buffer_to_send, conn_count);

  if (send_flag)
    DEBUGS("Command sent over to the server successfully");
  else {
    perror("Unable to send command");
    return;
  }

  for (i = 0, error_flag = false; i < conn_count; i++) {
    if (conn_fds[i] == -1)
      continue;
    recv_int_value_socket(conn_fds[i], &c);
    if (c == -1) {
      DEBUGS("Some Error has occured");
      error_flag = true;
      fetch_and_print_error(conn_fds[i]);
    }
  }

  if (error_flag)
    return;
  if (flag == LIST_FLAG) {

    DEBUGS("Fetching remote file(s) info from all the servers");
    mod = fetch_remote_file_info(conn_fds, conn_count, &server_chunks_collate);

    DEBUGS("Printing the file names and folders with status");
    get_output_list_command(&server_chunks_collate);

    // Fetching and printing folder names
    fetch_remote_dir_info(conn_fds, conn_count);

  } else if (flag == GET_FLAG) {

    // Fetch the file into from remote server, server_chunks_collate only has info of one file across all servers
    DEBUGS("Fetching remote file(s) info from all the servers");
    mod = fetch_remote_file_info(conn_fds, conn_count, &server_chunks_collate);

    if (mod < 0) {
      return;
    }
    /*print_server_chunks_collate_struct(&server_chunks_collate);*/

    DEBUGS("Checking whether the file is complete");
    if (!check_complete((&server_chunks_collate)->chunks[0])) {
      // In case file is incomplete
      // Send RESET SIG to server to terminate the connections
      printf("<<< File is incomplete\n");

      DEBUGS("Sending REST_SIG to server");
      send_signal(conn_fds, conn_count, (u_char)RESET_SIG);
    } else {

      // Send proceed signal to servers and request for files

      DEBUGS("File can can be fetched");

      DEBUGS("Sending PROCEED_SIG to server");
      send_signal(conn_fds, conn_count, (u_char)PROCEED_SIG);
      /*file_split.file_name = strdup(attr->local_file_name);*/

      DEBUGS("Fetching remote splits from the server");
      fetch_remote_splits(conn_fds, conn_count, &file_split, mod);

      DEBUGS("Decrypting the file splits");
      encrypt_decrypt_file_split(&file_split, conf->user->password);
      DEBUGS("Combining all the splits and writing into the file");
      // Combine the files on local path
      // Handles the case where file_split weren't written successfully given folder doesn't exist
      combine_file_from_pieces(attr, &file_split);
      /*print_file_split_struct(&file_split);*/
    }

  } else if (flag == PUT_FLAG) {

    DEBUGS("Getting mod value on file-content");
    // Generating local_file path to the file
    sprintf(file_path, "%s%s", attr->local_file_folder, attr->local_file_name);
    mod = get_md5_sum_hash_mod(file_path);

    DEBUGS("Splitting file into pieces");
    // TODO: handle case where this isn't successfuly
    split_file_to_pieces(file_path, &file_split, conn_count);

    DEBUGS("Encrypting the file splits");
    encrypt_decrypt_file_split(&file_split, conf->user->password);
    DEBUGS("Sending splits to servers");

    for (i = 0; i < conn_count; i++) {
      if (conn_fds[i] == -1)
        continue;
      send_file_splits(conn_fds[i], &file_split, mod, i);
    }
    DEBUGS("Splits sent to servers");
    free_file_split_struct(&file_split);
  } else if (flag == MKDIR_FLAG) {
  }
}

void fetch_remote_dir_info(int* conn_fds, int conn_count)
{
  int i, payload_size;
  bool flag = false;
  u_char* payload;
  for (i = 0; i < conn_count; i++) {
    if (conn_fds[i] == -1)
      continue;

    recv_int_value_socket(conn_fds[i], &payload_size);

    payload = (u_char*)malloc(payload_size * sizeof(u_char));
    recv_from_socket(conn_fds[i], payload, payload_size);

    flag = (!flag) ? printf("%s", payload) : flag;

    free(payload);
  }
}
bool check_complete(bool* flag_array)
{
  // Checks if chunks of file available are enough to reconstruct the file
  int sum, i;
  for (i = 0, sum = 0; i < NUM_SERVER; i++)
    sum += flag_array[i];
  return (sum == NUM_SERVER) ? true : false;
}

void get_output_list_command(server_chunks_collate_struct* server_chunks_collate)
{
  int i;
  for (i = 0; i < server_chunks_collate->num_files; i++) {
    printf("%s", server_chunks_collate->file_names[i]);
    if (check_complete(server_chunks_collate->chunks[i]))
      printf("\n");
    else
      printf(" [INCOMPLETE]\n");
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

void read_dfc_conf(char* file_path, dfc_conf_struct* conf)
{

  /* Reading configurations from the config file
   */
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
  //DEBUGSN("File Size read", (int)file_size);

  split_size = file_size / n; // First n - 1 splits will have split_size number of bytes
  rem_size = file_size % n;   // Last nth split will have split_size + rem_size number of bytes
  file_split->split_count = 0;
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

bool combine_file_from_pieces(file_attr_struct* file_attr, file_split_struct* file_split)
{
  FILE* fp;
  int i;
  split_struct* split;
  char file_name[MAXFILEBUFF];

  memset(file_name, 0, sizeof(file_name));
  sprintf(file_name, "%s%s", file_attr->local_file_folder, file_attr->local_file_name);

  DEBUGSS("Writing to file", file_name);
  if ((fp = fopen(file_name, "wb")) <= 0) {
    printf("<<< Unable to open file to write: %s", strerror(errno));
    return false;
  }

  for (i = 0; i < file_split->split_count; i++) {
    split = file_split->splits[i];
    fwrite(split->content, sizeof(u_char), split->content_length, fp);
  }
  fclose(fp);

  return true;
}

void print_dfc_conf_struct(dfc_conf_struct* conf)
{
  int i;
  dfc_server_struct* ptr;
  //DEBUGSS("Username", conf->user->username);
  //DEBUGSS("Password", conf->user->password);
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
