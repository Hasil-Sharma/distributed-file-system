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

bool auth_dfs_user(int socket, dfs_conf_struct* conf)
{
  char buffer[MAX_SEG_SIZE];
  int r_bytes, i;
  user_struct user;
  memset(buffer, 0, sizeof(buffer));
  if ((r_bytes = recv(socket, buffer, MAX_SEG_SIZE, 0)) <= 0) {
    perror("Failed to Authenticate Connection");
    return false;
  }
  user.username = (char*)malloc(MAXFILEBUFF * sizeof(char));
  user.password = (char*)malloc(MAXFILEBUFF * sizeof(char));
  decode_user_struct(buffer, &user);
  for (i = 0; i < conf->user_count; i++) {
    if (compare_user_struct(&user, conf->users[i])) {
      DEBUGSS("Authenticated", user.username);
      return true;
    }
  }
  DEBUGSS("Couldn't Authenticate", user.username);
  free(user.username);
  free(user.password);
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
  memset(&st, 0, sizeof(st));

  if (stat(path, &st) == -1)
    mkdir(path, 0755);
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
