#ifndef _UR_COMMIT_H
#define _UR_COMMIT_H

#include <time.h>

#include "list.h"

#define TYPE_BITS 2

#define TREE_TYPE 0x1
#define BLOB_TYPE 0x2


struct commit 
{
  time_t date;
  unsigned char parent_sha1_1[20];
  unsigned char parent_sha1_2[20];
  
  unsigned object_type : TYPE_BITS;
  unsigned char object_sha1[20];
  char *msg;

  struct list_elem elem;
};


/*
 * creates a new object for commit
 */
int commit_objectify (struct commit *commit, unsigned char sha1[20]);

/*
 * reads a commit from an object sha1
 */
int commit_read (struct commit *commit, const unsigned char sha1[20]);

/*
 * initialize commit with given parameter. set date.
 */
int commit_create (struct commit *commit, 
		   const unsigned char parent_sha1_1[20],
		   const unsigned char parent_sha1_2[20],
		   const unsigned object_type,
		   const unsigned char object_sha1[20],
		   const char *msg);
		   


#endif
