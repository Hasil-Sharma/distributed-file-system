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

enum DFSConstants {
  MAXUSERS = 10,
  MAXCONNECTION = 10
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
void read_dfs_conf(char*, dfs_conf_struct*);
void insert_dfs_user_conf(char*, dfs_conf_struct*);
void dfs_command_accept(int, dfs_conf_struct*);
bool dfs_command_decode_and_auth(char*, const char*, dfs_recv_command_struct*, dfs_conf_struct*);
bool dfs_command_exec(dfs_recv_command_struct*, dfs_conf_struct*, int);
void create_dfs_directory(char*);
void dfs_directory_creator(char*, dfs_conf_struct*);
void print_dfs_conf_struct(dfs_conf_struct*);
void free_dfs_conf_struct(dfs_conf_struct*);
#endif
