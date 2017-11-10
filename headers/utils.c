#include "utils.h"

bool check_file_exists(char* directory, char* file_name)
{
  char file_path_buffer[2 * MAXCHARBUFF];
  memset(file_path_buffer, 0, sizeof(file_path_buffer));

  sprintf(file_path_buffer, "%s%s", directory, file_name);
  return (access(file_path_buffer, F_OK) != -1) ? true : false;
}

bool check_directory_exists(char* path)
{
  struct stat st;
  return (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) ? true : false;
}
void encrypt_decrypt_file_split(file_split_struct* file_split, char* key)
{
  split_struct* split;
  int i, j, keylen;

  keylen = strlen(key);
  for (i = 0; i < file_split->split_count; i++) {

    split = file_split->splits[i];
    for (j = 0; j < split->content_length; j++) {
      split->content[j] = split->content[j] ^ key[i % keylen];
    }
  }
}
char* get_sub_string(char* haystack, char* needle)
{
  /* Searching for needle in haystack
   * Returns pointer to the first character where match happens
   */
  //Return NULL if either of them is NULL
  if (haystack == NULL || needle == NULL)
    return NULL;

  return strstr(haystack, needle);
}

void insert_to_server_chunks_collate_struct(server_chunks_collate_struct* server_chunks_collate, server_chunks_info_struct* server_chunks_info)
{
  int i, j, k, chunk_num;
  for (i = 0; i < server_chunks_info->chunks; i++) {

    j = check_file_name_exist(server_chunks_collate->file_names, server_chunks_info->chunk_info[i].file_name, server_chunks_collate->num_files);
    // Checking if file name received exits already

    if (j < 0) {
      // file doesn't exist

      server_chunks_collate->num_files++;
      j = server_chunks_collate->num_files - 1;
      strcpy(server_chunks_collate->file_names[j], server_chunks_info->chunk_info[i].file_name);
    }

    // file exits

    for (k = 0; k < CHUNKS_PER_SERVER; k++) {
      chunk_num = server_chunks_info->chunk_info[i].chunks[k];
      server_chunks_collate->chunks[j][chunk_num - 1] = true;
    }
  }
}

void read_into_split_from_file(char* file_path, split_struct* split)
{
  FILE* fp;
  struct stat sb;
  long long file_size;

  if ((fp = fopen(file_path, "rb")) <= 0) {
    fprintf(stderr, "Unable to open file: %s\n", file_path);
  }

  if (stat(file_path, &sb) == -1) {
    fprintf(stderr, "Error in getting file size: %s\n", file_path);
  }

  file_size = (long long)sb.st_size;

  split->content_length = file_size;
  split->content = (u_char*)malloc(file_size * sizeof(u_char));

  if (fread(split->content, sizeof(u_char), split->content_length, fp) < 0)
    perror("Error in reading split to split struct");
}

int check_file_name_exist(char file_names[][100], char* file_name, int n)

{
  int i;
  for (i = 0; i < n; i++)
    if (compare_str(file_names[i], file_name))
      return i;
  return -1;
}

bool get_files_in_folder(char* folder, server_chunks_info_struct* server_chunks, char* check_file_name)
{
  /* Returns info all the  files in a folder if check_file_name is NULL
   * Returns info of file named check_file_name if is it specified
   */

  int i, chunk_num, chunk_idx, j, folder_strlen = strlen(folder);
  char temp_folder[folder_strlen + 2], file_name[MAXCHARBUFF], *temp_ptr_1 = NULL, *temp_ptr_2 = NULL;
  bool file_found = false;
  memset(temp_folder, 0, sizeof(temp_folder));
  memset(file_name, 0, sizeof(file_name));
  strcpy(temp_folder, folder);

  temp_folder[folder_strlen] = '.';
  temp_folder[folder_strlen + 1] = '*';

  temp_folder[folder_strlen + 2] = '\0';
  /*DEBUGSS("Changed Path", temp_folder);*/
  glob_t glob_result;
  if (glob(temp_folder, GLOB_PERIOD, NULL, &glob_result) == GLOB_ERR) {
    perror("Error in Glob");
  }

  /*DEBUGSN("Num files", glob_result.gl_pathc);*/
  assert(glob_result.gl_pathc % 2 == 0);
  server_chunks->chunks = glob_result.gl_pathc / 2 - 1; // removing '.' and '..'
  if (check_file_name != NULL)
    server_chunks->chunks = 1;
  server_chunks->chunk_info = (chunk_info_struct*)malloc(server_chunks->chunks * sizeof(chunk_info_struct));

  for (i = 0, chunk_idx = -1; i < glob_result.gl_pathc; i++) {
    temp_ptr_1 = get_file_name_pointer_from_path(glob_result.gl_pathv[i]);
    chunk_num = 0;
    // To remove the case of "." and ".."
    if (strlen(temp_ptr_1) < 3)
      continue;
    /*DEBUGSS("File", temp_ptr_1);*/
    temp_ptr_2 = strrchr(temp_ptr_1, '.');
    chunk_num = atoi(temp_ptr_2 + 1);
    /*DEBUGSN("Chunk Num", chunk_num);*/
    *temp_ptr_2 = NULL_CHAR;
    /*DEBUGSS("File Name", temp_ptr_1 + 1);*/

    // Handle case when folder doesn't exits or check_file_name doesn't exit in specified folder
    // For GET command skip all the other files
    if (check_file_name != NULL && !compare_str(temp_ptr_1 + 1, check_file_name)) {
      continue;
    }

    if (!compare_str(temp_ptr_1 + 1, file_name)) {
      strcpy(file_name, temp_ptr_1 + 1);
      /*DEBUGSS("File Name set to", file_name);*/
      chunk_idx++;
      j = 0;
    }

    file_found = true;
    strcpy(server_chunks->chunk_info[chunk_idx].file_name, file_name);
    server_chunks->chunk_info[chunk_idx].chunks[j++] = chunk_num;
  }
  return file_found;
}

void print_server_chunks_info_struct(server_chunks_info_struct* server_chunks)
{
  DEBUGS("Printing Print Server Chunks Info Struct");
  int i;
  for (i = 0; i < server_chunks->chunks; i++) {
    print_chunks_info_struct(&server_chunks->chunk_info[i]);
  }
}

void print_chunks_info_struct(chunk_info_struct* chunk_info)
{
  DEBUGS("Printing chunk info struct");
  int i;
  DEBUGSS("Filename", chunk_info->file_name);
  for (i = 0; i < CHUNKS_PER_SERVER; i++)
    DEBUGSN("Chunk Number", chunk_info->chunks[i]);
}
bool compare_str(char* str1, char* str2)
{
  if (str1 == NULL || str2 == NULL)
    return false;
  return (strcmp(str1, str2) == 0) ? true : false;
}
char* get_token(char* string, char* delim, int offset)
{
  // Will only parse two token strings offset = 0 gives first string and offset = 1 give second string

  char* ptr = get_sub_string_after(string, delim);
  int len;

  if (ptr == NULL) {
    if (strlen(string) != 0 && offset == 0) // Case when string is sent without second token mentioned
      return strdup(string);
    else
      return NULL;
  }
  if (offset == 1)
    return strdup(ptr);

  ptr += offset;
  len = ptr - string - 1;
  return strndup(string, len);
}
char* get_sub_string_after(char* haystack, char* needle)
{
  char* ptr;
  if (haystack == NULL || needle == NULL)
    return NULL;
  ptr = strstr(haystack, needle);
  ptr += (ptr) ? strlen(needle) : 0; // Pointer now points to string after needle has compeletely occurred
  return ptr;
}

void extract_file_name_and_folder(char* buffer, file_attr_struct* file_attr, int flag)
{
  /* Extract file names and folder from the buffer sent and folders end with "/" in all the cases
   * Saves to remote/local folder/file depending on the flag
   */
  char* ptr;
  int temp_len;
  /*DEBUGSS("Buffer to extract from", buffer);*/
  ptr = get_file_name_pointer_from_path(buffer);

  if (flag == EXTRACT_LOCAL) {

    // Copy into local variable names
    if (ptr) {
      // File name exist AFTER a folder path
      strcpy(file_attr->local_file_name, ptr);
      temp_len = ptr - buffer;
      strncpy(file_attr->local_file_folder, buffer, temp_len);
      /*DEBUGSS("Local file name", file_attr->local_file_name);*/
      /*DEBUGSS("Local file folder", file_attr->local_file_folder);*/
    } else {
      // File name does not exist AFTER a folder path
      if ((ptr = get_sub_string(buffer, ROOT_FOLDER_STR)) != 0) {
        // Buffer is only folder name
        strcpy(file_attr->local_file_folder, buffer);
        /*DEBUGSS("Local file folder", file_attr->local_file_folder);*/
      } else {
        // Buffer is only file name
        strcpy(file_attr->local_file_name, buffer);
        /*DEBUGSS("Local file name", file_attr->local_file_name);*/
      }
    }

  } else if (flag == EXTRACT_REMOTE) {

    // Copy into remote variable names
    if (ptr) {
      // File name exits AFTER a folder path
      strcpy(file_attr->remote_file_name, ptr);
      temp_len = ptr - buffer;
      strncpy(file_attr->remote_file_folder, buffer, temp_len);
      /*DEBUGSS("Remote file name", file_attr->remote_file_name);*/
      /*DEBUGSS("Remote file folder", file_attr->remote_file_folder);*/
    } else {
      // File name does not exist AFTER a folder path
      if ((ptr = get_sub_string(buffer, ROOT_FOLDER_STR)) != 0) {
        // Buffer is only folder name
        strcpy(file_attr->remote_file_folder, buffer);
        /*DEBUGSS("Remote file folder", file_attr->remote_file_folder);*/
      } else {
        // Buffer is only file name
        strcpy(file_attr->remote_file_name, buffer);
        /*DEBUGSS("Remote file name", file_attr->remote_file_name);*/
      }
    }
  }
}

void write_split_struct_to_file(split_struct* split, char* file_folder, char* file_name)
{
  // file_folder has a '/' in the end

  FILE* fp;
  char file_path[2 * MAXCHARBUFF];
  memset(file_path, 0, sizeof(file_path));

  sprintf(file_path, "%s.%s.%d", file_folder, file_name, split->id);

  DEBUGSS("File written at", file_path);
  if ((fp = fopen(file_path, "wb")) == NULL) {
    perror("Error in opening file to write");
    return;
  }
  if (fwrite(split->content, sizeof(u_char), split->content_length, fp) != split->content_length) {
    perror("Error in writing split to file");
    fclose(fp);
    return;
  }

  fclose(fp);
}
int get_count_str_chr(char* buffer, char chr)
{
  int count = 0, i;
  if (buffer == NULL)
    return 0;

  for (i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == chr)
      count++;
  }
  return count;
}
char* get_file_name_pointer_from_path(char* buffer)
{

  /* Assummes buffer is of type folder1/folder2/file_name
   * Return pointer to the first character of file name
   */
  char* ptr;
  int buffer_len;
  if (buffer == NULL)
    return NULL;
  buffer_len = strlen(buffer);
  ptr = strrchr(buffer, ROOT_FOLDER_CHR);
  if (ptr == NULL || ptr - buffer + 1 == buffer_len) // Last Character is "/"
    return NULL;
  return ++ptr;
}

bool compare_user_struct(user_struct* u1, user_struct* u2)
{
  return (strcmp(u1->username, u2->username) == 0 && strcmp(u1->password, u2->password) == 0) ? true : false;
}
bool check_user_struct(user_struct** user)
{
  if (*user == NULL) {
    *user = (user_struct*)malloc(sizeof(user_struct));
    return false;
  }
  return true;
}

void free_user_struct(user_struct* user)
{
  free(user->username);
  free(user->password);
}

int get_md5_sum_hash_mod(char* file_path)
{
  u_char c[MD5_DIGEST_LENGTH], data[MAXFILEBUFF];
  FILE* in_file = fopen(file_path, "rb");

  if (in_file <= 0) {
    fprintf(stderr, "Unable to open file: %s", file_path);
    return -1;
  }
  MD5_CTX md_context;
  int bytes, i, mod;

  MD5_Init(&md_context);
  memset(data, 0, sizeof(data));
  while ((bytes = fread(data, 1, MAXFILEBUFF, in_file)) != 0)
    MD5_Update(&md_context, data, bytes);
  MD5_Final(c, &md_context);
  /*fprintf(stderr, "DEBUG: md5sum value of file ");*/

  for (i = 0, mod = 0; i < MD5_DIGEST_LENGTH; i++) {
    /*fprintf(stderr, "%02x", c[i]);*/
    mod = (mod * 16 + (u_int)c[i]) % NUM_SERVER;
  }
  /*fprintf(stderr, " Mod: %d", mod);*/
  /*fprintf(stderr, "\n");*/
  DEBUGSN("MOD:", mod);
  return mod;
}

void print_hash_value(u_char* buffer, int length)
{
  int i;
  u_char c[MD5_DIGEST_LENGTH];
  MD5_CTX context;
  MD5_Init(&context);
  MD5_Update(&context, buffer, length);
  MD5_Final(c, &context);

  for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    fprintf(stderr, "%02x", c[i]);

  fprintf(stderr, "\n");
}

void free_file_split_struct(file_split_struct* file_split)
{
  free(file_split->file_name);
  while (--file_split->split_count) {
    free_split_struct(file_split->splits[file_split->split_count]);
  }
}

void free_split_struct(split_struct* split)
{
  free(split->content);
}

void print_file_split_struct(file_split_struct* file_split)
{
  int i = 0;
  DEBUGS("Printing File Split Struct");
  DEBUGSS("Filename", file_split->file_name);
  DEBUGSN("Number of splits", file_split->split_count);
  for (i = 0; i < file_split->split_count; i++) {
    print_split_struct(file_split->splits[i]);
    DEBUGS("");
  }
}

void print_split_struct(split_struct* split)
{
  DEBUGSN("Split with id", split->id);
  DEBUGSN("Content_length", split->content_length);
  DEBUGSS("Content", (char*)split->content);
}

void print_server_chunks_collate_struct(server_chunks_collate_struct* server_chunks_collate)
{
  int i, j;
  DEBUGS("Printing Server Chunks Collate Struct");
  DEBUGSN("Num File", server_chunks_collate->num_files);
  for (i = 0; i < server_chunks_collate->num_files; i++) {
    DEBUGSS("File name", server_chunks_collate->file_names[i]);
    for (j = 0; j < NUM_SERVER; j++) {
      DEBUGSN("Chunk", j + 1);
      DEBUGSN("Present", server_chunks_collate->chunks[i][j]);
    }
  }
}
