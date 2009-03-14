#ifndef _UR_COMMIT_H
#define _UR_COMMIT_H

#include <time.h>
#include <stdbool.h>

#include "list.h"
#include "ur.h"

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

  bool alive;

  struct list_elem elem;
};


extern struct commit COMMIT_INITIALIZER;

/*
 * initialize commit module
 */
int init_commit ();

/*
 * creates a new object for commit
 */
int commit_objectify (state_t *ur, struct commit *commit, unsigned char sha1[20]);

/*
 * reads a commit from an object sha1
 */
int commit_read (state_t *ur, struct commit *commit, const unsigned char sha1[20]);

/*
 * initialize commit with given parameter. set date.
 */
int commit_create (struct commit *commit, 
		   const unsigned char parent_sha1_1[20],
		   const unsigned char parent_sha1_2[20],
		   const unsigned object_type,
		   const unsigned char object_sha1[20],
		   const char *msg);
		   
/*
 * destroy internall commit data
 */
int commit_destroy (struct commit *commit);

#endif
