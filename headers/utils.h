#include "debug.h"
#include <openssl/md5.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  MAXFILEBUFF = 100,
  MAXSERVERS = 10,
  NUM_SERVER = 4
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

char* get_sub_string(char*, char*);
char* get_token(char*, char*, int);
char* get_sub_string_after(char*, char*);
bool check_user_struct(user_struct**);
void free_user_struct(user_struct*);
int get_md5_sum_hash_mod(char*);
void free_file_split_struct(file_split_struct*);
void free_split_struct(split_struct*);
void print_file_split_struct(file_split_struct*);
void print_split_struct(split_struct*);
#endif
