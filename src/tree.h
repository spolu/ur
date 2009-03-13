#ifndef _UR_TREE_H
#define _UR_TREE_H

#include <stdbool.h>

#include "object.h"
#include "list.h"


struct blob_tree_entry 
{
  char *name;
  char *commit;

  struct list_elem elem;  
};


struct branch_tree_entry
{
  char *name;
  char *branch;
  
  struct list_elem elem;
};

struct tree 
{
  struct list blob_entries;
  struct list branch_entries;

  bool alive;
};


extern struct tree TREE_INITIALIZER;

/*
 *
 */
int init_tree ();

/*
 * creates a new object for tree.
 */
int tree_objectify (struct tree *tree, unsigned char sha1[20]);


/*
 * reads tree from object
 */
int tree_read (struct tree *tree, unsigned char sha1[20]);


/*
 * initialize an empty tree
 */
int tree_init (struct tree *tree);


/*
 * destroy tree internal data
 */
int tree_destroy (struct tree *tree);

/*
 * add blob_tree_entry to tree. idempotent
 * replace name if already existing
 */
int tree_blob_entry_add (struct tree *tree, char *name, char *commit);

/*
 * add branch_tree_entry to tree. idempotent.
 * replace name if already existing
 */
int tree_branch_entry_add (struct tree *tree, char *name, char *branch);


/*
 * removes any entry associated with name
 */
int tree_remove_entry (struct tree *tree, char *name);

#endif
