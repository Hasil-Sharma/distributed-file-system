#include "dfsutils.h"

void read_dfs_conf(char* file_path, dfs_conf_struct* conf)
{
  FILE* fp;
  char line[MAXFILEBUFF];
  int line_len, i;
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
