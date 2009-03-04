#ifndef _COMMIT_H
#define _COMMIT_H

#include "object.h"

struct commit_list {
  struct commit *item;
  struct commit_list *next;
};

struct commit_map {
  unsigned int nr;
  unsigned int alloc;
  struct commit_map_entry {
    struct commit *item;
    const char *name;
  } *commits;
};

struct commit {
  struct object object;
  unsigned long date;
  struct commit_list *parents;  //commits
  struct object *object;        //tree or blob
  char *buffer;
};

#endif
