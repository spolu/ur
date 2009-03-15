#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

#include "helper.h"


int
subdir_check (const char *root, const char *path)
{
  char * subdir = NULL;
  struct stat64 st_buf;

  subdir = (char *) malloc (strlen (root) + strlen (path) + 2);
  if (subdir == NULL)
    return -1;

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

  sprintf (subdir, "%s/%s", root, path);
  
  if (mkdir (subdir, S_IRWXU | S_IRGRP | S_IWGRP) != 0) goto error;

  free (subdir);

  return 0;
  
 error:
  if (subdir != NULL) free (subdir);

  return -1;
}


int
dirname (const char *path, char *name)
{
  int i = 0;
  struct stat64 st_buf;  
  char * path_cpy;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  path_cpy = (char *) malloc (strlen (path) + 1);
  if (path_cpy == NULL) goto error;  
  strncpy (path_cpy, path, strlen (path) + 1);

  if (strcmp (path_cpy, "/") == 0 ||
      strcmp (path_cpy, ".") == 0) {
    strncpy (name, path_cpy, strlen (path_cpy) + 1);    
    free (path_cpy); path_cpy = NULL;

    //printf ("dirname: %s -> %s\n", path, name);
    return 0;
  }
  
  if (path_cpy[strlen (path_cpy) - 1] == '/') {
    path_cpy [strlen (path_cpy) - 1] = 0;
  }
  
  for (i = strlen (path_cpy) - 1; i >= 0; i --) {
    if (path_cpy[i] == '/')
      break;
  }
  
  if (i < 0) {
    strncpy (name, path_cpy, strlen (path) + 1);
  }
  
  else {
    strncpy (name, (path_cpy+(i+1)), strlen ((path_cpy+(i+1))) + 1);
  }

  free (path_cpy); path_cpy = NULL;  

  //printf ("dirname: %s -> %s\n", path, name);
  return 0;

 error:
  if (path_cpy != NULL) free (path_cpy);
  return -1;
}

int
filename (const char *path, char *name)
{
  int i = 0;
  struct stat64 st_buf;  
  const char * path_cpy;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  if (path[strlen (path) - 1] == '/') {
    goto error;
  }

  for (i = strlen (path) - 1; i >= 0; i --) {
    if (path[i] == '/')
      break;
  }
  
  if (i < 0) {
    strncpy (name, path, strlen (path) + 1);
  }

  else {
    path_cpy = path+(i+1);
    strncpy (name, path_cpy, strlen (path_cpy) + 1);
  }
  
  //printf ("filename: %s -> %s\n", path, name);
  return 0;

 error:
  return -1;
}



int 
parent_dir (const char *path, char *parent)
{
  int i = 0;
  struct stat64 st_buf;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  if (strcmp (path, "/") == 0) {
    sprintf (parent, "/");
    return 0;
  }
    

  strncpy (parent, path, strlen (path) + 1);
  if (parent[strlen (parent) - 1] == '/') {
    parent[strlen (parent) - 1] = 0;
  }

  for (i = strlen (parent) - 1; i >= 0; i --) {
    if (parent[i] == '/') {
      parent[i] = 0;
      break;
    }
  }
  
  if (i == 0) {
    sprintf (parent, "/");
  }

  if (i < 0)
    {
      if (st_buf.st_mode & S_IFREG) {
	sprintf (parent, ".");
      }
      
      if (st_buf.st_mode & S_IFDIR) {
	sprintf (parent, "%s/..", parent);
      }
    }

  //printf ("prt_dir : %s -> %s\n", path, parent);

  return 0;
  
 error:
  return -1;
}

