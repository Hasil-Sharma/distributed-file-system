#include "debug.h"
#include <assert.h>
#include <glob.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifndef UTILS_H
#define UTILS_H

#define mkzero(obj) memset(&obj, 0, sizeof(obj))
#define NEW_LINE_CHAR '\n'
#define NULL_CHAR '\0'
#define SPACE_STR " "
#define COLON_STR ":"
#define ROOT_FOLDER_STR "/"
#define ROOT_FOLDER_CHR '/'

enum Constants {
  EXTRACT_LOCAL = 0,
  EXTRACT_REMOTE = 1,
  CHUNKS_PER_SERVER = 2,
  MAXFILEBUFF = 100,
  MAXSERVERS = 10,
  NUM_SERVER = 4,
  MAXCHARBUFF = 100,
  MAX_NUM_FILES = 100
};

typedef struct user_struct {
  char* username;
  char* password;
} user_struct;

typedef struct split_struct {
  int id;
  u_char* content;
  int content_length;
} split_struct;

typedef struct file_split_struct {
  char* file_name;
  split_struct* splits[NUM_SERVER];
  int split_count;
} file_split_struct;

typedef struct file_attr_struct {
  char remote_file_name[MAXCHARBUFF];
  char remote_file_folder[MAXCHARBUFF];
  char local_file_name[MAXCHARBUFF];
  char local_file_folder[MAXCHARBUFF];
} file_attr_struct;

typedef struct chunk_info_struct {
  char file_name[MAXCHARBUFF];
  int chunks[CHUNKS_PER_SERVER];
} chunk_info_struct;

typedef struct server_chunks_info_struct {
  int chunks;
  chunk_info_struct* chunk_info;
} server_chunks_info_struct;

typedef struct server_chunks_collate_struct {
  char file_names[MAX_NUM_FILES][MAXCHARBUFF];
  bool chunks[MAX_NUM_FILES][NUM_SERVER];
  int num_files;
} server_chunks_collate_struct;

char* get_sub_string(char*, char*);
char* get_token(char*, char*, int);
char* get_sub_string_after(char*, char*);
bool compare_str(char*, char*);
bool check_complete(bool*);
int check_file_name_exist(char file_names[][100], char*, int);
char* get_file_name_pointer_from_path(char*);
void get_files_in_folder(char*, server_chunks_info_struct*, char*);
void extract_file_name_and_folder(char*, file_attr_struct*, int);
void print_chunks_info_struct(chunk_info_struct*);
void print_server_chunks_info_struct(server_chunks_info_struct*);
void insert_to_server_chunks_collate_struct(server_chunks_collate_struct*, server_chunks_info_struct*);
int get_count_str_chr(char*, char);
bool compare_user_struct(user_struct*, user_struct*);
bool check_user_struct(user_struct**);
void free_user_struct(user_struct*);
int get_md5_sum_hash_mod(char*);
void read_into_split_from_file(char*, split_struct*);
void print_hash_value(u_char*, int);
void write_split_struct_to_file(split_struct*, char*, char*);
void free_file_split_struct(file_split_struct*);
void free_split_struct(split_struct*);
void print_file_split_struct(file_split_struct*);
void print_split_struct(split_struct*);
void print_server_chunks_collate_struct(server_chunks_collate_struct*);
#endif
