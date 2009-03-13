#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "cmd-add.h"


int
cmd_add (const char *path, bool recursive)
{
  struct stat64 st_buf;
  char * prt_dir = NULL;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  prt_dir = (char *) malloc (strlen (path) + 4);
  parent_dir (path, prt_dir);

  if (st_buf.st_mode & S_IFREG) 
    {
      
    }

  if (st_buf.st_mode & S_IFDIR) 
    {
      if (state_check (path) == 0)
	{
	  if (state_check (prt_dir) == 0) {
	    // update prt_dir index
	  }
	  else {
	    printf ("%s already initialized", path);
	    goto error;
	  }
	}
      else
	{
	  if (state_init (path) != 0) {
	    printf ("%s initialization failed\n", path);
	    goto error;
	  }
  
	  printf ("%s initialized\n", path);
	  
	  if (state_check (prt_dir) == 0) {
	    // update prt_dir index
	  }	  
	}
    }

  free (prt_dir);
  
  return 0;

 error:
  if (prt_dir != NULL)
    free (prt_dir);
  return -1;
}

int 
parent_dir (const char *path, char * parent)
{
  struct stat64 st_buf;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  if (strcmp (path, "/") == 0) {
    sprintf (parent, "/");
    return 0;
  }
    

  int i = 0;
  strncpy (parent, path, strlen (path));
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

  printf ("prt_dir : %s -> %s\n", path, parent);

  return 0;
  
 error:
  return -1;
}

