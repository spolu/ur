#ifndef _UR_BRANCH_H
#define _UR_BRANCH_H

#include "ur.h"
#include "tree.h"

/*
 * read the commit sha
 */
int branch_read_commit_sha1 (state_t *ur, unsigned char sha1[20], const char *branchname);

/*
 * read tree to which branch is pointing
 */
int branch_read_tree (state_t *ur, struct tree *tree, const char *branchname);

/*
 * returns HEAD branch name
 */ 
char * branch_get_head_name (state_t *ur);


#endif
