#ifndef _UR_TREE_H
#define _UR_TREE_H

#include "object.h"
#include "list.h"


struct blob_tree_entry 
{
  const char *name;
  const char *commit;

  struct list_elem elem;  
};


struct branch_tree_entry
{
  const char *name;
  const char *branch;
  
  struct list_elem elem;
};

struct tree 
{
  struct list blob_entries;
  struct list branch_entries;
};



/*
 * Creates a new object for tree.
 */
int tree_objectify (struct tree *tree, unsigned char *sha1[20]);


/*
 * Reads tree from object
 */
int tree_read (struct tree *tree, unsigned char sha1[20]);


#endif
