#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef DFSUTILS_H
#define DFSUTILS_H

enum DFSConstants {
  MAXUSERS = 10
};

typedef struct dfs_conf_struct {
  user_struct* users[MAXUSERS];
  int user_count;
} dfs_conf_struct;

void read_dfs_conf(char*, dfs_conf_struct*);
void insert_dfs_user_conf(char*, dfs_conf_struct*);
void print_dfs_conf_struct(dfs_conf_struct*);
void free_dfs_conf_struct(dfs_conf_struct*);
#endif
