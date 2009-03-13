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

struct state ur_state;

int
init_ur ()
{
  init_commit ();
  init_tree ();

  return 0;
}


int 
state_check (const char *path)
{
  int fd = -1;
  char * head = NULL;
  unsigned char sha1[20];
  struct commit head_commit = COMMIT_INITIALIZER;
  struct tree head_tree = TREE_INITIALIZER;

  if (subdir_check (path, UR_DIR) != 0)
    goto error;
  
  if (subdir_check (path, UR_DIR_HEADS) != 0)
    goto error;

  if (subdir_check (path, UR_DIR_SHADOWS) != 0)
    goto error;
      
  if (file_check (path, UR_INDEX) != 0)
    goto error;

  if (file_check (path, UR_LOCK) != 0)
    goto error;

  if (file_check (path, UR_HEAD) != 0)
    goto error;

  if ((fd = file_open (path, UR_HEAD, O_RDONLY)) < 0)
    goto error;

  head = readline (fd);
  close (fd);

  if (head == NULL)
    goto error;
  
  if ((fd = file_open (path, head, O_RDONLY)) < 0)
    goto error;

  free (head);
  head = NULL;

  head = readline (fd);
  hex_to_sha1 (head, sha1);

  free (head);
  head = NULL;
  close (fd);
  
  /*
   * used by commit_read
   */
  ur_state.path = path;
  ur_state.branches = NULL;
  ur_state.branch = NULL;
  ur_state.head = NULL;

  if (commit_read (&head_commit, sha1) < 0)
    goto error;

  memcpy (sha1, head_commit.object_sha1, 20);
  commit_destroy (&head_commit);

  if (tree_read (&head_tree, sha1) < 0)
    goto error;

  tree_destroy (&head_tree);

  return 0;

 error:
  if (head != NULL)
    free (head);
  commit_destroy (&head_commit);
  tree_destroy (&head_tree);

  return -1;
}


int 
state_init (const char *path)
{
  
  int fd = -1;
  char * head = NULL;
  unsigned char null_sha1[20];
  struct commit initial_commit = COMMIT_INITIALIZER;
  struct tree empty_tree = TREE_INITIALIZER;
  unsigned char sha1[20];
  char buf[50];

  if (subdir_create (path, UR_DIR) != 0)
    goto error;
  
  if (subdir_create (path, UR_DIR_HEADS) != 0)
    goto error;

  if (subdir_create (path, UR_DIR_SHADOWS) != 0)
    goto error;

  if (subdir_create (path, UR_DIR_OBJECTS) != 0)
    goto error;

  if ((fd = file_open (path, UR_INDEX, O_WRONLY | O_TRUNC | O_CREAT)) < 0)
    goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  close (fd);
  
  if ((fd = file_open (path, UR_LOCK, O_WRONLY | O_TRUNC | O_CREAT)) < 0)
    goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  close (fd);

  if ((fd = file_open (path, UR_HEAD, O_WRONLY | O_TRUNC | O_CREAT)) < 0)
    goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);

  head = (char *) malloc (strlen (UR_DIR_HEADS) + strlen (UR_MASTER) + 2);
  sprintf (head, "%s/%s", UR_DIR_HEADS, UR_MASTER);  
  writeline (fd, head, strlen (head), "\n");
  close (fd);
  free (head);
  head = NULL;

  /*
   * used by commit_objectify
   */
  ur_state.path = path;
  ur_state.branches = NULL;
  ur_state.branch = NULL;
  ur_state.head = NULL;

  memset (null_sha1, 0, 20);

  if (tree_init (&empty_tree) != 0)
    goto error;

  if (tree_objectify (&empty_tree, sha1) != 0)
    goto error;  

  tree_destroy (&empty_tree); //NOP

  if (commit_create (&initial_commit, 
		     null_sha1, 
		     null_sha1, 
		     TREE_TYPE, 
		     sha1, 
		     "initial commit") != 0)
    goto error;

  if (commit_objectify (&initial_commit, sha1) != 0)
    goto error;

  commit_destroy (&initial_commit);
  
  head = (char *) malloc (strlen (UR_DIR_HEADS) + strlen (UR_MASTER) + 2);
  sprintf (head, "%s/%s", UR_DIR_HEADS, UR_MASTER);  

  if ((fd = file_open (path, head, O_WRONLY | O_TRUNC | O_CREAT)) < 0)
    goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);

  sha1_to_hex (sha1, buf);
  writeline (fd, buf, 40, "\n");
  close (fd);
  free (head);
  head = NULL;

  return 0;

 error:
  if (head != NULL)
    free (head);
  commit_destroy (&initial_commit);
  tree_destroy (&empty_tree);

  return -1;
}



int
subdir_check (const char *root, const char *path)
{
  char * subdir = NULL;
  struct stat64 st_buf;

  subdir = (char *) malloc (strlen (root) + strlen (path) + 2);
  if (subdir == NULL)
    return -1;

  if (strlen (root) == 0 || path[strlen (root) - 1] == '/')
    sprintf (subdir, "%s%s", root, path);
  else
    sprintf (subdir, "%s/%s", root, path);
  
  if (lstat64 (subdir, &st_buf) != 0) 
    goto error;

  if (!(st_buf.st_mode & S_IFDIR))
    goto error;

  if (!(st_buf.st_mode & S_IRWXU))
    goto error;

  free (subdir);

  return 0;
  
 error:
  if (subdir != NULL)
    free (subdir);
  return -1;
}


int
file_check (const char *root, const char *path)
{
  char * file = NULL;
  struct stat64 st_buf;

  file = (char *) malloc (strlen (root) + strlen (path) + 2);
  if (file == NULL)
    return -1;

  if (strlen (root) == 0 || path[strlen (root) - 1] == '/')
    sprintf (file, "%s%s", root, path);
  else
    sprintf (file, "%s/%s", root, path);
  
  if (lstat64 (file, &st_buf) != 0) 
    goto error;

  if (!(st_buf.st_mode & S_IFREG))
    goto error;

  if (!(st_buf.st_mode & S_IRUSR))
    goto error;

  if (!(st_buf.st_mode & S_IWUSR))
    goto error;

  free (file);

  return 0;
  
 error:
  if (file != NULL)
    free (file);  
  return -1;
}


int
file_open (const char *root, const char *path, int oflag)
{
  char * file = NULL;
  int fd = -1;
  
  file = (char *) malloc (strlen (root) + strlen (path) + 2);
  if (file == NULL)
    return -1;

  if (strlen (root) == 0 || path[strlen (root) - 1] == '/')
    sprintf (file, "%s%s", root, path);
  else
    sprintf (file, "%s/%s", root, path);
  
  if ((fd = open (file, oflag)) < 0)
    goto error;
  
  free (file);
  
  return fd;
  
 error:
  if (file != NULL)
    free (file);    
  return -1;
}


int
subdir_create (const char *root, const char *path)
{
  char * subdir = NULL;

  subdir = (char *) malloc (strlen (root) + strlen (path) + 2);
  if (subdir == NULL)
    return -1;

  if (strlen (root) == 0 || path[strlen (root) - 1] == '/')
    sprintf (subdir, "%s%s", root, path);
  else
    sprintf (subdir, "%s/%s", root, path);
  
  if (mkdir (subdir, S_IRWXU | S_IRGRP | S_IWGRP) != 0)
    goto error;

  free (subdir);

  return 0;
  
 error:
  if (subdir != NULL)
    free (subdir);
  return -1;
}
