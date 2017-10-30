#include "dfcutils.h"

int main(int argc, char** argv)
{
  dfc_conf_struct conf;
  char *conf_file, *file_name, *remote_folder, buffer[MAXFILEBUFF], *char_ptr;
  int buffer_size, mod;
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

    if (buffer[buffer_size - 1] == NEW_LINE_CHAR)
      buffer[--buffer_size] = NULL_CHAR;

    // Assumption is that folders always end with '/' and file names are absolute

    if ((remote_folder = get_sub_string_after(buffer, DFC_LIST_CMD))) {

      DEBUGSS("Command Sent is LIST", remote_folder);
      remote_folder = strlen(remote_folder) ? remote_folder : strdup(ROOT_FOLDER_STR);
      DEBUGSS("Remote Folder", remote_folder);
      free(remote_folder);

    } else if ((char_ptr = get_sub_string_after(buffer, DFC_GET_CMD))) {

      DEBUGSS("Command Sent is GET", char_ptr);
      file_name = get_token(char_ptr, SPACE_STR, 0);
      remote_folder = get_token(char_ptr, file_name, 1);
      remote_folder = strlen(remote_folder) ? remote_folder : strdup(ROOT_FOLDER_STR);
      DEBUGSS("File name", file_name);
      DEBUGSS("Remote Folder", remote_folder);
      mod = get_md5_sum_hash_mod(file_name);
      free(file_name);
      free(remote_folder);
      if (mod < 0)
        continue;

    } else if ((char_ptr = get_sub_string_after(buffer, DFC_PUT_CMD))) {

      DEBUGSS("Command Sent is PUT", char_ptr);
      file_name = get_token(char_ptr, SPACE_STR, 0);
      remote_folder = get_token(char_ptr, file_name, 1);
      remote_folder = strlen(remote_folder) ? remote_folder : strdup(ROOT_FOLDER_STR);
      DEBUGSS("File name", file_name);
      DEBUGSS("Remote Folder", remote_folder);
      mod = get_md5_sum_hash_mod(file_name);
      if (mod < 0)
        continue;
      split_file_to_pieces(file_name, &file_split, NUM_SERVER);
      /*print_file_split_struct(&file_plit);*/
      free_file_split_struct(&file_split);
      free(file_name);
      free(remote_folder);

    } else if ((remote_folder = get_sub_string_after(buffer, DFC_MKDIR_CMD))) {

      DEBUGSS("Command Sent is MKDIR", remote_folder);
      if (strlen(remote_folder) == 0)
        continue;
      DEBUGSS("Remote Folder", remote_folder);
      free(remote_folder);
    }
    memset(buffer, 0, sizeof(buffer));
  }
  free_dfc_conf_struct(&conf);
  return 0;
}
