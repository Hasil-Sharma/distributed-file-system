#include "dfsutils.h"

int main(int argc, char** argv)
{
  dfs_conf_struct conf;
  char *server_folder, *file_name = "conf/dfs.conf", *response;
  int port_number, listen_fd, conn_fd, r_bytes;
  struct sockaddr_in remote_address;
  socklen_t addr_size = sizeof(struct sockaddr_in);
  mkzero(conf);
  mkzero(conf.users);
  if (argc != 3) {
    fprintf(stderr, "USAGE: dfs <folder> <port>\n");
    exit(1);
  }

  server_folder = argv[1];
  port_number = atoi(argv[2]);

  read_dfs_conf(file_name, &conf);
  // Assumption that server_folder begins with a /
  /*dfs_directory_creator(++server_folder, &conf);*/

  listen_fd = get_dfs_socket(port_number);

  while (true) {
    DEBUGSS("Waiting to Accept Connection", server_folder);
    if ((conn_fd = accept(listen_fd, (struct sockaddr*)&remote_address, &addr_size)) <= 0) {
      perror("Error Accepting Connection");
      continue;
    }

    while (true)
      dfs_command_accept(conn_fd, &conf);
    /*response = AUTH_OK;*/
    /*DEBUGS("Connection Accepted");*/
    /*if (!auth_dfs_user(conn_fd, &conf)) {*/
    /*DEBUGSS("Failed to Authenticate on", server_folder);*/
    /*response = AUTH_NOT_OK;*/
    /*}*/

    /*if ((r_bytes = send(conn_fd, response, MAX_SEG_SIZE, 0)) <= 0) {*/
    /*perror("Cannot respond with AUTH OK");*/
    /*continue;*/
    /*}*/

    /*if(!get_sub_string(response, AUTH_OK))*/
  }
  print_dfs_conf_struct(&conf);
  free_dfs_conf_struct(&conf);
  return 0;
}
