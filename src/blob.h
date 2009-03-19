#ifndef _UR_BLOB_H
#define _UR_BLOB_H

struct tree;

/*
 * initializes blob module
 */
int init_blob ();

/*
 * creates a new object for blob designed by a the file descriptor.
 */
int blob_objectify (state_t *ur, int fd, unsigned char sha1[20]);

/*
 * commits blob represented by name
 * using ptree as parent
 * updating tree
 */
int commit_blob_using_tree (state_t *ur, 
			    struct tree *tree, 
			    char *name,
			    struct tree *ptree,
			    char *msg);


#endif
