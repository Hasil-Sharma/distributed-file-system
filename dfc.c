#include "dfcutils.h"

int main(int argc, char** argv)
{
  dfc_conf_struct conf;
  char *conf_file, buffer[MAXFILEBUFF], *char_ptr;
  int buffer_size, *conn_fds;
  file_split_struct file_split;

  memset(&conf, 0, sizeof(conf));
  memset(&conf.servers, 0, sizeof(conf.servers));

  if (argc != 2) {
    fprintf(stderr, "USAGE: dfc <conf_file>\n");
    exit(1);
  }
  conf_file = argv[1];
  read_dfc_conf(conf_file, &conf);
  /*print_dfc_conf_struct(&conf);*/

  while (true) {

    memset(buffer, 0, sizeof(buffer));
    memset(&file_split, 0, sizeof(file_split));
    memset(file_split.splits, 0, sizeof(file_split.splits));

    fprintf(stdout, ">>> ");
    fgets(buffer, MAXFILEBUFF, stdin);
    buffer_size = strlen(buffer);

    /*DEBUGS("Setting up connections with remote servers");*/
    /*setup_dfc_to_dfs_connections(&conn_fds, &conf);*/

    conn_fds = (int*)malloc(conf.server_count * sizeof(int));
    if (buffer[buffer_size - 1] == NEW_LINE_CHAR)
      buffer[--buffer_size] = NULL_CHAR;

    // Assumption is that folders always end with '/' and file names are absolute

    if ((char_ptr = get_sub_string_after(buffer, DFC_LIST_CMD))) {

      DEBUGSS("Command Sent is LIST", char_ptr);
      dfc_command_handler(conn_fds, LIST_FLAG, char_ptr, &conf);

    } else if ((char_ptr = get_sub_string_after(buffer, DFC_GET_CMD))) {

      DEBUGSS("Command Sent is GET", char_ptr);
      dfc_command_handler(conn_fds, GET_FLAG, char_ptr, &conf);

    } else if ((char_ptr = get_sub_string_after(buffer, DFC_PUT_CMD))) {

      DEBUGSS("Command Sent is PUT", char_ptr);
      dfc_command_handler(conn_fds, PUT_FLAG, char_ptr, &conf);

    } else if ((char_ptr = get_sub_string_after(buffer, DFC_MKDIR_CMD))) {

      DEBUGSS("Command Sent is MKDIR", char_ptr);
      dfc_command_handler(conn_fds, MKDIR_FLAG, char_ptr, &conf);
    } else {
      DEBUGSS("Invalid Command", buffer);
    }
    memset(buffer, 0, sizeof(buffer));

    free(conn_fds);
  }
  free_dfc_conf_struct(&conf);
  return 0;
}
