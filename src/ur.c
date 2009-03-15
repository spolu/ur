#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ur.h"
#include "object.h"
#include "io.h"
#include "commit.h"
#include "sha1.h"
#include "tree.h"
#include "helper.h"
#include "index.h"

state_t STATE_INITIALIZER;

int
init_ur ()
{
  init_commit ();
  init_tree ();
  init_index ();
  memset (&STATE_INITIALIZER, 0, sizeof (state_t));
  return 0;
}


int
state_destroy (state_t *ur)
{  
  if (ur->alive) {
    if (ur->path != NULL) { free ((char *) ur->path); ur->path = NULL; }
  }
  ur->alive = false;
  return 0;
}


int
state_init (state_t *ur, const char *path)
{
  char * path_cpy = NULL;

  state_destroy (ur);

  path_cpy = (char *) malloc (strlen (path) + 1);
  if (path_cpy == NULL) goto error;
  strncpy (path_cpy, path, strlen (path) + 1);
  ur->path = path_cpy;  
  ur->alive = true;

  return 0;

 error:
  state_destroy (ur);
  return -1;
}


int 
ur_check (const char *path)
{
  int fd = -1;
  char * head = NULL;
  unsigned char sha1[20];
  struct commit head_commit = COMMIT_INITIALIZER;
  struct tree head_tree = TREE_INITIALIZER;
  state_t ur = STATE_INITIALIZER;

  ur.path = path;
  ur.alive = true;

  if (subdir_check (path, UR_DIR) != 0) goto error;  
  if (subdir_check (path, UR_DIR_HEADS) != 0) goto error;
  if (subdir_check (path, UR_DIR_SHADOWS) != 0) goto error;      
  if (file_check (path, UR_INDEX) != 0) goto error;
  if (file_check (path, UR_LOCK) != 0) goto error;
  if (file_check (path, UR_HEAD) != 0) goto error;

  if ((fd = file_open (path, UR_HEAD, O_RDONLY)) < 0) goto error;
  head = readline (fd);
  close (fd);
  if (head == NULL) goto error;
  
  if ((fd = file_open (path, head, O_RDONLY)) < 0) goto error;
  free (head); head = NULL;

  head = readline (fd);
  hex_to_sha1 (head, sha1);
  free (head); head = NULL;
  close (fd);
  
  if (commit_read (&ur, &head_commit, sha1) < 0) goto error;
  memcpy (sha1, head_commit.object_sha1, 20);
  commit_destroy (&head_commit);

  if (tree_read (&ur, &head_tree, sha1) < 0) goto error;
  tree_destroy (&head_tree);

  return 0;

 error:
  if (head != NULL) free (head);
  commit_destroy (&head_commit);
  tree_destroy (&head_tree);

  return -1;
}


int 
ur_create (const char *path)
{
  
  int fd = -1;
  char * head = NULL;
  unsigned char null_sha1[20];
  struct commit initial_commit = COMMIT_INITIALIZER;
  struct tree empty_tree = TREE_INITIALIZER;
  unsigned char sha1[20];
  char buf[50];
  state_t ur = STATE_INITIALIZER;

  ur.path = path;
  ur.alive = true;

  if (subdir_create (path, UR_DIR) != 0) goto error;  
  if (subdir_create (path, UR_DIR_HEADS) != 0) goto error;
  if (subdir_create (path, UR_DIR_SHADOWS) != 0) goto error;
  if (subdir_create (path, UR_DIR_OBJECTS) != 0) goto error;

  if ((fd = file_open (path, UR_INDEX, O_WRONLY | O_TRUNC | O_CREAT)) < 0) goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  sprintf (buf, "0");
  writeline (fd, buf, strlen (buf), "\n");
  close (fd);
  
  if ((fd = file_open (path, UR_LOCK, O_WRONLY | O_TRUNC | O_CREAT)) < 0) goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  close (fd);

  if ((fd = file_open (path, UR_HEAD, O_WRONLY | O_TRUNC | O_CREAT)) < 0) goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  head = (char *) malloc (strlen (UR_DIR_HEADS) + strlen (UR_MASTER) + 2);
  sprintf (head, "%s/%s", UR_DIR_HEADS, UR_MASTER);  
  writeline (fd, head, strlen (head), "\n");
  close (fd);
  free (head); head = NULL;

  memset (null_sha1, 0, 20);

  if (tree_init (&empty_tree) != 0) goto error;
  if (tree_objectify (&ur, &empty_tree, sha1) != 0) goto error;  
  tree_destroy (&empty_tree); //NOP

  if (commit_create (&initial_commit, 
		     null_sha1, 
		     null_sha1, 
		     TREE_TYPE, 
		     sha1, 
		     "initial commit") != 0) goto error;

  if (commit_objectify (&ur, &initial_commit, sha1) != 0) goto error;
  commit_destroy (&initial_commit);
  
  head = (char *) malloc (strlen (UR_DIR_HEADS) + strlen (UR_MASTER) + 2);
  sprintf (head, "%s/%s", UR_DIR_HEADS, UR_MASTER);  

  if ((fd = file_open (path, head, O_WRONLY | O_TRUNC | O_CREAT)) < 0) goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  sha1_to_hex (sha1, buf);
  writeline (fd, buf, 40, "\n");
  close (fd);
  free (head); head = NULL;

  return 0;

 error:
  if (head != NULL) free (head);
  commit_destroy (&initial_commit);
  tree_destroy (&empty_tree);

  return -1;
}

