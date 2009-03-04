#ifndef _TREE_H
#define _TREE_H

#include "object.h"
#include "commit.h"

struct tree {
  struct object object;
  struct commit_map *map;  //name->commit mappings
};

#endif
