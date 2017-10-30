#include "dfsutils.h"

int main(int argc, char** argv)
{
  dfs_conf_struct conf;
  char* server_folder;
  char* file_name = "conf/dfs.conf";
  int port_number;
  mkzero(conf);
  mkzero(conf.users);
  if (argc != 3) {
    fprintf(stderr, "USAGE: dfs <folder> <port>\n");
    exit(1);
  }

  server_folder = argv[1];
  port_number = atoi(argv[2]);

  read_dfs_conf(file_name, &conf);
  print_dfs_conf_struct(&conf);
  free_dfs_conf_struct(&conf);
  return 0;
}
