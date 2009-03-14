#ifndef _UR_TREE_H
#define _UR_TREE_H

#include <stdbool.h>

#include "object.h"
#include "list.h"
#include "ur.h"

struct blob_tree_entry 
{
  char *name;
  unsigned char commit[20];

  struct list_elem elem;  
};


struct branch_tree_entry
{
  char *name;
  char *branch;
  unsigned char commit[20];
  
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
 * tree module initialization
 */
int init_tree ();

/*
 * creates a new object for tree.
 */
int tree_objectify (state_t *ur, struct tree *tree, unsigned char sha1[20]);


/*
 * reads tree from object
 */
int tree_read (state_t *ur, struct tree *tree, unsigned char sha1[20]);


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
int tree_blob_entry_add (struct tree *tree, char *name, unsigned char commit[20]);

/*
 * add branch_tree_entry to tree. idempotent.
 * replace name if already existing
 */
int tree_branch_entry_add (struct tree *tree, char *name, char *branch, unsigned char commit[20]);


/*
 * removes any entry associated with name
 */
int tree_entry_remove (struct tree *tree, char *name);

#endif
