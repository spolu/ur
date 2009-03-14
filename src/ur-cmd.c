#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "ur-cmd.h"
#include "helper.h"
#include "ur.h"
#include "index.h"

int
cmd_add (const char *path, bool recursive)
{
  char *prt_dir = NULL, *name = NULL;
  struct stat64 st_buf;
  state_t ur = STATE_INITIALIZER;
  struct index index = INDEX_INITIALIZER; 
  int status;

  if (lstat64 (path, &st_buf) != 0) {
    printf ("%s does not exist\n", path);
    goto error;
  } 

  name = (char *) malloc (strlen (path) + 4);
  prt_dir = (char *) malloc (strlen (path) + 4);
  parent_dir (path, prt_dir);
  filename (path, name);	

  if (st_buf.st_mode & S_IFREG) {
    if (ur_check (prt_dir) == 0) 
      {
	state_init (&ur, prt_dir);
	index_read (&ur, &index);
	
	status = index_entry_get_status (&index, name);
	status |= S_IADD;
	if (!(status & S_IPST))
	  status |= S_IPST;
	
	index_entry_set_status (&index, name, status);
	
	index_write (&ur, &index);
	index_destroy (&index);
	state_destroy (&ur);
      }
  }

  if (st_buf.st_mode & S_IFDIR) 
    {
      if (ur_check (path) == 0) {
	if (ur_check (prt_dir) == 0) {
	  // update prt_dir index
	}
	else {
	  printf ("%s already initialized\n", path);
	  goto error;
	}
      }
      else {
	if (ur_create (path) != 0) {
	  printf ("%s initialization failed\n", path);
	  goto error;
	}
	
	printf ("%s initialized\n", path);
	
	if (ur_check (prt_dir) == 0) {
	  // update prt_dir index
	}	  
      }
    }
  
  free (prt_dir);
  free (name);

  /*
   * TODO recursion
   */
  
  return 0;

 error:
  if (prt_dir != NULL) free (prt_dir);
  if (name != NULL) free (name);

  return -1;
}


