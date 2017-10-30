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
  user_struct* users[MAXUSERS];
  int user_count;
} dfs_conf_struct;

int get_dfs_socket(int);
bool auth_dfs_user(int, dfs_conf_struct*);
void read_dfs_conf(char*, dfs_conf_struct*);
void insert_dfs_user_conf(char*, dfs_conf_struct*);
void create_dfs_directory(char*);
void dfs_directory_creator(char*, dfs_conf_struct*);
void print_dfs_conf_struct(dfs_conf_struct*);
void free_dfs_conf_struct(dfs_conf_struct*);
#endif
