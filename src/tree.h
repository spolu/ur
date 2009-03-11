#ifndef _UR_TREE_H
#define _UR_TREE_H

#include "object.h"
#include "list.h"

#define ENTRY_TYPE_BITS 2;
#define ENTRY_TYPE_BRANCH 0x1;
#define ENTRY_TYPE_BLOB 0x2;

struct tree_entry 
{
  const char *name;
  unsigned type : ENTRY_TYPE_BITS;

  const char *branch;
  const blob *blob;

  struct list_elem elem;  
};


struct tree 
{
  struct object object;
  struct list entries;
};

#endif
