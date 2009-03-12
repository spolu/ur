#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "object.h"
#include "io.h"

static int subdir_check (const char *path, const char *dir);
static int file_check (const char *root, const char *path);
static int subdir_create (const char *root, const char *path);
static int file_open (const char *root, const char *path, int oflag);


int 
state_check (const char *path)
{
  int fd = -1;
  char * head;
  
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

  if ((fd = file_open (path, UR_HEAD, O_RDWR)) < 0)
    goto error;

  head = readline (fd);
  close (fd);

  if (head == NULL)
    goto error;
  
  printf ("head found: %s\n", head);
  free (head);

  return 0;

 error:
  return -1;
}


int 
state_init (const char *path)
{
  
  int fd = -1;
  char * head;
  
  if (subdir_create (path, UR_DIR) != 0)
    goto error;
  
  if (subdir_create (path, UR_DIR_HEADS) != 0)
    goto error;

  if (subdir_create (path, UR_DIR_SHADOWS) != 0)
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

  return 0;

 error:
  return -1;
}



static int
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


static int
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


static int
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


static int
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
