#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>

#include "ur-cmd.h"
#include "helper.h"
#include "ur.h"
#include "index.h"

void
fail (const char *fmt, ...) 
{
  char str[1000];
  va_list ap;

  strncpy (str, fmt, 1000);

  va_start(ap, fmt);
  vsnprintf(str, 1000, fmt, ap);
  va_end(ap);

  printf ("ur: ");
  printf (str);
  printf ("\n");
  exit (-1);
}

void
output (const char *fmt, ...)
{
  char str[1000];
  va_list ap;

  strncpy (str, fmt, 1000);

  va_start(ap, fmt);
  vsnprintf(str, 1000, fmt, ap);
  va_end(ap);

  printf ("ur: ");
  printf (str);
  printf ("\n");
}


int
cmd_add (const char *path, bool recursive)
{
  char *prt_dir = NULL, *fname = NULL, *dname = NULL;
  struct stat64 st_buf;
  state_t ur = STATE_INITIALIZER;
  struct index index = INDEX_INITIALIZER; 
  int status;

  if (lstat64 (path, &st_buf) != 0) fail("%s does not exist", path);

  prt_dir = (char *) malloc (strlen (path) + 4);
  parent_dir (path, prt_dir);

  if (st_buf.st_mode & S_IFREG) {

    fname = (char *) malloc (strlen (path) + 4);
    filename (path, fname);	

    if (ur_check (prt_dir) == 0) 
      {
	if (state_init (&ur, prt_dir) != 0) fail ("fail reading state of %s", prt_dir);
	if (index_read (&ur, &index) != 0) fail ("fail reading index of %s", prt_dir);
	
	status = index_entry_get_status (&index, fname);
	status |= S_IADD;
	if (!(status & S_IPST))
	  status |= S_IPST;
	index_entry_set_status (&index, fname, status);

	if (index_write (&ur, &index) != 0) fail ("fail writing index of %s", prt_dir);

	index_destroy (&index);
	state_destroy (&ur);
      }
    else
      fail ("%s initialization failed", prt_dir);	

    free (fname); fname = NULL;    
  }

  if (st_buf.st_mode & S_IFDIR) 
    {
      dname = (char *) malloc (strlen (path) + 4);
      dirname (path, dname);	

      if (ur_check (path) == 0) {
	if (ur_check (prt_dir) == 0) 
	  {
	    if (state_init (&ur, prt_dir) != 0) fail ("fail reading state of %s", prt_dir);
	    if (index_read (&ur, &index) != 0) fail ("fail reading index of %s", prt_dir);
	    
	    status = index_entry_get_status (&index, dname);
	    status |= S_IADD;
	    if (!(status & S_IPST))
	      status |= S_IPST;	
	    index_entry_set_status (&index, dname, status);

	    if (index_write (&ur, &index) != 0) fail ("fail writing index of %s", prt_dir);
	    index_destroy (&index);
	    state_destroy (&ur);
	}
      }
      else {

	if (ur_create (path) != 0) fail ("%s initialization failed", path);	
	output ("init: %s", path);
	
	if (ur_check (prt_dir) == 0) {
	    if (state_init (&ur, prt_dir) != 0) fail ("fail reading state of %s", prt_dir);
	    if (index_read (&ur, &index) != 0) fail ("fail reading index of %s", prt_dir);
	    
	    status = index_entry_get_status (&index, dname);
	    status |= S_IADD;
	    if (!(status & S_IPST))
	      status |= S_IPST;	
	    index_entry_set_status (&index, dname, status);

	    if (index_write (&ur, &index) != 0) fail ("fail writing index of %s", prt_dir);
	    index_destroy (&index);
	    state_destroy (&ur);
	}
      }

      free (dname); dname = NULL;    
    }
  
  free (prt_dir); prt_dir = NULL;

  /*
   * recursion
   */
  if ((st_buf.st_mode & S_IFDIR) && recursive) 
    {
      DIR *dp;
      struct dirent *ep;
      
      dp = opendir (path);
      if (dp != NULL)
	{
	  while ((ep = readdir (dp))) {
	    if (ep->d_name[0] != '.') 
	      {		
		char *npath;
		npath = (char *) malloc (strlen (path) +
					 strlen (ep->d_name) + 2);
		if (path[strlen (path) -1] == '/')
		  sprintf (npath, "%s%s", path, ep->d_name);
		else
		  sprintf (npath, "%s/%s", path, ep->d_name);
		cmd_add (npath, recursive);
		free (npath);
	      }	    
	  }
	  (void) closedir (dp);
	}
      else
	fail ("could not open directory %s", path);      
    }
  
  return 0;
}


int
cmd_status (const char *path, bool recursive)
{
  return 0;
}
