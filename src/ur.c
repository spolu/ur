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

  ur_state.path = NULL;
  ur_state.alive = false;

  return 0;
}


int
state_destroy ()
{
  char * path_cpy = (char *) ur_state.path;
  if (ur_state.path != NULL) { free (path_cpy); ur_state.path = NULL; }
  index_destroy ();

  return 0;
}


int
state_init (const char *path)
{
  char * path_cpy = NULL;

  state_destroy ();
  path_cpy = (char *) malloc (strlen (path) + 1);
  if (path_cpy == NULL) goto error;
  strncpy (path_cpy, path, strlen (path));
  ur_state.path = path_cpy;  
  if (index_read () != 0) goto error;

  return 0;

 error:
  state_destroy ();
  return -1;
}


int
index_read ()
{
  int fd = -1;
  char * buf = NULL, *name = NULL;
  int cnt, i;
  int dirty, added;
  time_t ctime;
  struct tm tm;
  struct index_entry *index_entry = NULL;
  char *cp;

  index_destroy ();
  list_init (&ur_state.index);

  if ((fd = file_open (ur_state.path, UR_INDEX, O_RDONLY)) < 0)
    goto error;
  
  buf = readline (fd);
  if (buf == NULL) goto error;
  cnt = atoi (buf);
  free (buf); buf = NULL;
  
  for (i = 0; i < cnt; i ++) 
    {
      buf = readline (fd);
      if (buf == NULL) goto error;
      name = buf;
      buf = NULL;
  
      buf = readline (fd);
      if (buf == NULL) goto error;
      dirty = atoi (buf);
      free (buf); buf = NULL;
      
      buf = readline (fd);
      if (buf == NULL) goto error;
      added = atoi (buf);
      free (buf);
      free (buf); buf = NULL;
      
      buf = readline (fd);  
      if (buf == NULL) goto error;
      cp = strptime (buf, "%a %b %d %T %Y", &tm);
      ctime = mktime (&tm);
      free (buf); buf = NULL;
      
      index_entry = (struct index_entry *) malloc (sizeof (struct index_entry));
      if (index_entry == NULL)
	goto error;

      index_entry->name = name;
      index_entry->dirty = dirty;
      index_entry->added = added;
      index_entry->ctime = ctime;

      list_push_back (&ur_state.index, &index_entry->elem);

      name = NULL;
      index_entry = NULL;
    }

  close (fd);
  ur_state.alive = true;

  return 0;

 error:
  if (buf != NULL) free (buf);
  if (name != NULL) free (name);
  if (index_entry != NULL) free (index_entry);

  ur_state.alive = true;
  index_destroy ();

  return -1;
}


int
index_write ()
{
  struct list_elem *e;
  char buf[50];
  int fd = -1;
  char * ct;

  if (!ur_state.alive) goto error;
  
  if ((fd = file_open (ur_state.path, UR_INDEX, O_TRUNC | O_WRONLY)) < 0)
    goto error;
  
  sprintf (buf, "%d", (int) list_size (&ur_state.index));
  writeline (fd, buf, strlen (buf), "\n");

    for (e = list_begin (&ur_state.index); e != list_end (&ur_state.index);
       e = list_next (e))
      {
	struct index_entry *en = list_entry (e, struct index_entry, elem);	
	writeline (fd, en->name, strlen (en->name), "\n");
	sprintf (buf, "%d", en->dirty);
	writeline (fd, buf, strlen (buf), "\n");	
	sprintf (buf, "%d", en->added);
	writeline (fd, buf, strlen (buf), "\n");	
	ct = ctime (&en->ctime);
	writeline (fd, ct, strlen (ct) - 1, "\n");      	
      }

 error:
  return -1;
}

int
index_destroy ()
{
  if (ur_state.alive) 
    {
      while (list_size (&ur_state.index) > 0)
	{
	  struct list_elem *e = list_front (&ur_state.index);
	  list_remove (e);
	  struct index_entry *en = list_entry (e, struct index_entry, elem);
	  free (en->name);
	  free (en);
	}
    }
  
  ur_state.alive = false;

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

  state_destroy ();

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
  
  /*
   * used by commit_read
   */
  ur_state.path = path;

  if (commit_read (&head_commit, sha1) < 0) goto error;
  memcpy (sha1, head_commit.object_sha1, 20);
  commit_destroy (&head_commit);

  if (tree_read (&head_tree, sha1) < 0) goto error;
  tree_destroy (&head_tree);
  ur_state.path = NULL;

  return 0;

 error:
  if (head != NULL) free (head);
  commit_destroy (&head_commit);
  tree_destroy (&head_tree);
  state_destroy ();

  return -1;
}


int 
state_create (const char *path)
{
  
  int fd = -1;
  char * head = NULL;
  unsigned char null_sha1[20];
  struct commit initial_commit = COMMIT_INITIALIZER;
  struct tree empty_tree = TREE_INITIALIZER;
  unsigned char sha1[20];
  char buf[50];

  state_destroy ();

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

  /*
   * used by commit_objectify
   */
  ur_state.path = path;

  memset (null_sha1, 0, 20);

  if (tree_init (&empty_tree) != 0) goto error;
  if (tree_objectify (&empty_tree, sha1) != 0) goto error;  
  tree_destroy (&empty_tree); //NOP

  if (commit_create (&initial_commit, 
		     null_sha1, 
		     null_sha1, 
		     TREE_TYPE, 
		     sha1, 
		     "initial commit") != 0) goto error;

  if (commit_objectify (&initial_commit, sha1) != 0) goto error;
  commit_destroy (&initial_commit);
  
  head = (char *) malloc (strlen (UR_DIR_HEADS) + strlen (UR_MASTER) + 2);
  sprintf (head, "%s/%s", UR_DIR_HEADS, UR_MASTER);  

  if ((fd = file_open (path, head, O_WRONLY | O_TRUNC | O_CREAT)) < 0) goto error;
  fchmod (fd, S_IRUSR | S_IWUSR | S_IRGRP);
  sha1_to_hex (sha1, buf);
  writeline (fd, buf, 40, "\n");
  close (fd);
  free (head); head = NULL;

  ur_state.path = NULL;

  return 0;

 error:
  if (head != NULL) free (head);
  commit_destroy (&initial_commit);
  tree_destroy (&empty_tree);
  state_destroy ();

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
  
  if (lstat64 (subdir, &st_buf) != 0) goto error;
  if (!(st_buf.st_mode & S_IFDIR)) goto error;
  if (!(st_buf.st_mode & S_IRWXU)) goto error;

  free (subdir);

  return 0;
  
 error:
  if (subdir != NULL) free (subdir);

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
  
  if (lstat64 (file, &st_buf) != 0) goto error;
  if (!(st_buf.st_mode & S_IFREG)) goto error;
  if (!(st_buf.st_mode & S_IRUSR)) goto error;
  if (!(st_buf.st_mode & S_IWUSR)) goto error;

  free (file);

  return 0;
  
 error:
  if (file != NULL) free (file);  

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
  
  if ((fd = open (file, oflag)) < 0) goto error;
  
  free (file);
  
  return fd;
  
 error:
  if (file != NULL) free (file);    
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
  
  if (mkdir (subdir, S_IRWXU | S_IRGRP | S_IWGRP) != 0) goto error;

  free (subdir);

  return 0;
  
 error:
  if (subdir != NULL) free (subdir);

  return -1;
}
