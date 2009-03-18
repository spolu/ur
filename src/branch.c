#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "tree.h"
#include "list.h"
#include "io.h"
#include "sha1.h"
#include "commit.h"
#include "object.h"
#include "helper.h"

int 
branch_read_commit_sha1 (state_t *ur, unsigned char sha1[20], const char *branchname)
{
  int fd = -1;
  struct commit head_commit = COMMIT_INITIALIZER;
  char *head = NULL, *buf = NULL;

  head = (char *) malloc (strlen (branchname) + strlen (UR_DIR_HEADS) + 2);
  if (head == NULL) goto error;  
  sprintf (head, "%s/%s", UR_DIR_HEADS, branchname);

  if ((fd = file_open (ur->path, head, O_RDONLY)) < 0) goto error;
  free (head); head = NULL;

  buf = readline (fd);
  if (buf == NULL) goto error;
  hex_to_sha1 (buf, sha1);
  free (buf); buf = NULL;
  close (fd);
  
  if (commit_read (ur, &head_commit, sha1) < 0) goto error;
  memcpy (sha1, head_commit.object_sha1, 20);
  commit_destroy (&head_commit);

  return 0;

 error:
  if (head != NULL) free (head);
  if (buf != NULL) free (buf);
  return -1;
  
}

int 
branch_read_tree (state_t *ur, struct tree *tree, const char *branchname)
{
  int fd = -1;
  unsigned char sha1[20];
  struct commit head_commit = COMMIT_INITIALIZER;
  char *head = NULL, *buf = NULL;

  head = (char *) malloc (strlen (branchname) + strlen (UR_DIR_HEADS) + 2);
  if (head == NULL) goto error;  
  sprintf (head, "%s/%s", UR_DIR_HEADS, branchname);

  if ((fd = file_open (ur->path, head, O_RDONLY)) < 0) goto error;
  free (head); head = NULL;

  buf = readline (fd);
  if (buf == NULL) goto error;
  hex_to_sha1 (buf, sha1);
  free (buf); buf = NULL;
  close (fd);
  
  if (commit_read (ur, &head_commit, sha1) < 0) goto error;
  memcpy (sha1, head_commit.object_sha1, 20);
  commit_destroy (&head_commit);

  if (tree_read (ur, tree, sha1) < 0) goto error;  

  return 0;

 error:
  if (head != NULL) free (head);
  if (buf != NULL) free (buf);
  return -1;
}


char *
branch_get_head_name (state_t *ur)
{
  int fd = -1;
  char *head = NULL, *branchname = NULL, *buf = NULL;

  if ((fd = file_open (ur->path, UR_HEAD, O_RDONLY)) < 0) goto error;
  head = readline (fd);
  close (fd);
  if (head == NULL) goto error;
  if (strlen (head) <= (strlen (UR_DIR_HEADS) + 1)) goto error;
  
  buf = head + (strlen (UR_DIR_HEADS) + 1);
  branchname = (char *) malloc (strlen (buf) + 1);
  if (branchname == NULL) goto error;
  strncpy (branchname, buf, strlen (buf) + 1);

  free (head); head = NULL;  

  return branchname;

 error:
  if (head != NULL) free (head);
  if (branchname != NULL) free (branchname);
  return NULL;  
} 
