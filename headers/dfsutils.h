#include "netutils.h"
#include "utils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#ifndef DFSUTILS_H
#define DFSUTILS_H

#define FOLDER_NOT_FOUND_ERROR "Requested folder does not exist"
#define FOLDER_EXISTS_ERROR "Requested folder already exists"
#define FILE_NOT_FOUND_ERROR "Requested file does not exits"
#define AUTH_FAILED_ERROR "Invalid Username/Password. Please try again"
enum DFSConstants {
  MAXUSERS = 10,
  MAXCONNECTION = 10,
  FOLDER_NOT_FOUND = 1,
  FOLDER_EXISTS = 2,
  FILE_NOT_FOUND = 3,
  AUTH_FAILED = 4
};

typedef struct dfs_conf_struct {
  char server_name[MAXCHARBUFF];
  user_struct* users[MAXUSERS];
  int user_count;
} dfs_conf_struct;

typedef struct dfs_recv_command_struct {
  int flag;
  user_struct user;
  char folder[MAXCHARBUFF]; // Folder always ends with "/" and doesn't begin with one
  char file_name[MAXCHARBUFF];
} dfs_recv_command_struct;

int get_dfs_socket(int);
bool auth_dfs_user(user_struct*, dfs_conf_struct*);
void send_error_helper(int, const char*);
void send_error(int, int);
void read_dfs_conf(char*, dfs_conf_struct*);
void insert_dfs_user_conf(char*, dfs_conf_struct*);
void dfs_command_accept(int, dfs_conf_struct*);
bool dfs_command_decode_and_auth(char*, const char*, dfs_recv_command_struct*, dfs_conf_struct*);
bool dfs_command_exec(int, dfs_recv_command_struct*, dfs_conf_struct*, int);
void create_dfs_directory(char*);
void dfs_directory_creator(char*, dfs_conf_struct*);
void print_dfs_conf_struct(dfs_conf_struct*);
void free_dfs_conf_struct(dfs_conf_struct*);
#endif
