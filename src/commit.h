#ifndef _UR_COMMIT_H
#define _UR_COMMIT_H

#include "object.h"
#include "tree.h"
#include "list.h"

#define TYPE_BITS 2

#define TREE_TYPE 0x1
#define BLOB_TYPE 0x2

struct commit 
{
  unsigned long date;
  unsigned char parent_sha1_1[20];
  unsigned char parent_sha1_2[20];
  
  unsigned object_type : TYPE_BITS;
  unsigned char object_sha1[20];
  char *msg;

  struct list_elem elem;
};


/*
 * Creates a new object for commit
 */
int commit_objectify (struct commit *commit, unsigned char *sha1[20]);

#endif
