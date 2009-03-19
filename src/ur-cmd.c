#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

#include "ur-cmd.h"
#include "helper.h"
#include "ur.h"
#include "index.h"
#include "debug.h"
#include "branch.h"
#include "blob.h"
#include "commit.h"
#include "sha1.h"
#include "io.h"

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
  state_t ur = STATE_INITIALIZER;
  struct index index = INDEX_INITIALIZER; 
  struct stat64 st_buf;
  DIR *dp;
  struct dirent *ep;
  struct list_elem *e;
  char *branchname = NULL;
      
  if (lstat64 (path, &st_buf) != 0) fail("%s does not exist", path);
  ASSERT (st_buf.st_mode & S_IFDIR);

  if (ur_check (path) == 0) 
    {
      
      if (state_init (&ur, path) != 0) fail ("fail reading state of %s", path);
      if (index_read (&ur, &index) != 0) fail ("fail reading index of %s", path);

      index_update (&ur, &index);

      branchname = branch_get_head_name (&ur);
      printf ("*** %s \n    (branch: %s)\n", ur.path, branchname);
      free (branchname); branchname = NULL;

      // reading added dirty files
      for (e = list_begin (&index.entries); e != list_end (&index.entries);
	   e = list_next (e))
	{
	  struct index_entry *en = list_entry (e, struct index_entry, elem);	
	  if ((en->status & S_IPST) &&
	      (en->status & S_IADD) && 
	      (en->status & S_IDRT)) {
	    printf ("#  added     : %s/%s\n", path, en->name);
	  }
	}

      // reading dirty but not added
      for (e = list_begin (&index.entries); e != list_end (&index.entries);
	   e = list_next (e))
	{
	  struct index_entry *en = list_entry (e, struct index_entry, elem);	
	  if ((en->status & S_IPST) &&
	      !(en->status & S_IADD) && 
	      (en->status & S_IDRT) && 
	      (en->status & S_ITRK)) {
	    printf ("#  dirty     : %s/%s\n", path, en->name);
	  }
	}

      // reading untracked files
      for (e = list_begin (&index.entries); e != list_end (&index.entries);
	   e = list_next (e))
	{
	  struct index_entry *en = list_entry (e, struct index_entry, elem);	
	  if ((en->status & S_IPST) &&
	      !(en->status & S_ITRK) && 
	      !(en->status & S_IADD)) {
	    printf ("#  untracked : %s/%s\n", path, en->name);
	  }
	}
      
      index_destroy (&index);
      state_destroy (&ur);    
    }
  else {
    printf ("*** %s \n    (not initialized)\n", path);
    return 0;
  }

  /*
   * recursion
   */
  if (recursive) 
    {      
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

		if (lstat64 (npath, &st_buf) != 0) fail("%s does not exist", npath);
		if (st_buf.st_mode & S_IFDIR)
		  cmd_status (npath, recursive);
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
cmd_commit (const char *path, bool recursive, bool all, char *msg)
{
  state_t ur = STATE_INITIALIZER;
  struct index index = INDEX_INITIALIZER; 
  struct tree tree = TREE_INITIALIZER;
  struct tree ptree = TREE_INITIALIZER;
  struct commit commit = COMMIT_INITIALIZER;
  struct stat64 st_buf;
  DIR *dp;
  struct dirent *ep;
  struct list_elem *e;
  char *branchname = NULL, *npath = NULL;
  unsigned char sha1[20];
  unsigned char psha1[20];
  char buf[50];
  unsigned char null_sha1[20];
  int fd = -1, status;

  memset (null_sha1, 0, 20);

  if (lstat64 (path, &st_buf) != 0) fail("%s does not exist", path);
  ASSERT (st_buf.st_mode & S_IFDIR);

  /*
   * recursion [FIRST HERE]
   */
  if (recursive) 
    {      
      dp = opendir (path);
      if (dp != NULL)
	{
	  while ((ep = readdir (dp))) {
	    if (ep->d_name[0] != '.') 
	      {		
		npath = (char *) malloc (strlen (path) +
					 strlen (ep->d_name) + 2);
		if (path[strlen (path) -1] == '/')
		  sprintf (npath, "%s%s", path, ep->d_name);
		else
		  sprintf (npath, "%s/%s", path, ep->d_name);

		if (lstat64 (npath, &st_buf) != 0) fail("%s does not exist", npath);
		if (st_buf.st_mode & S_IFDIR)
		  cmd_commit (npath, recursive, all, msg);
		free (npath); npath = NULL;
	      }	    
	  }
	  (void) closedir (dp);
	}
      else
	fail ("could not open directory %s", path);      
    }  

  if (ur_check (path) == 0) 
    {      
      if (state_init (&ur, path) != 0) fail ("fail reading state of %s", path);
      if (index_read (&ur, &index) != 0) fail ("fail reading index of %s", path);

      index_update (&ur, &index);

      branchname = branch_get_head_name (&ur);
      printf ("*** %s \n    (branch: %s)\n", ur.path, branchname);

       // we start by setting the tree to its parent state
      if (branch_read_tree (&ur, &tree, branchname) != 0)
	fail ("could not read tree for %s (branch: %s)", ur.path, branchname);

      // keeps the parent tree around
      if (branch_read_tree (&ur, &ptree, branchname) != 0)
	fail ("could not read tree for %s (branch: %s)", ur.path, branchname);

      if (branch_read_commit_sha1 (&ur, psha1, branchname) != 0)
	fail ("could not read branch commit for %s (branch: %s)", ur.path, branchname);


      // removing non present / untracked files / branch 
      for (e = list_begin (&index.entries); e != list_end (&index.entries);
	   e = list_next (e))
	{
	  struct index_entry *en = list_entry (e, struct index_entry, elem);	
	  if (!(en->status & S_IPST) ||
	      ((en->status & S_IPST) &&
	       !(en->status & S_ITRK) && 
	       !(en->status & S_IADD))) {
	    printf ("removing : %s\n", en->name);
	    tree_entry_remove (&tree, en->name);
	  }
	}


      // commiting added dirty files / branch
      for (e = list_begin (&index.entries); e != list_end (&index.entries);
	   e = list_next (e))
	{
	  struct index_entry *en = list_entry (e, struct index_entry, elem);	
	  if ((en->status & S_IPST) &&
	      (en->status & S_IADD) && 
	      (en->status & S_IDRT)) 
	    {
	      char *npath;
	      npath = (char *) malloc (strlen (path) +
				       strlen (en->name) + 2);
	      if (path[strlen (path) -1] == '/')
		sprintf (npath, "%s%s", path, en->name);
	      else
		sprintf (npath, "%s/%s", path, en->name);
	      
	      if (lstat64 (npath, &st_buf) != 0) fail("%s does not exist", npath);
	      if (st_buf.st_mode & S_IFDIR) {
		if (commit_branch_using_tree (&ur, &tree, en->name) != 0)
		  fail ("%s commit failed", npath);		  
	      }
	      if (st_buf.st_mode & S_IFREG) {
		if (commit_blob_using_tree (&ur, &tree, en->name, &ptree, msg) != 0)
		  fail ("%s commit failed", npath);
	      }

	      status = index_entry_get_status (&index, en->name);
	      status &= ~S_IADD;
	      index_entry_set_status (&index, en->name, status);
	      
	      printf ("commit: %s\n", npath);
	      free (npath); npath = NULL;
	  }
	}

      // commiting dirty but not added file if all
      if (all) 
	{
	  for (e = list_begin (&index.entries); e != list_end (&index.entries);
	       e = list_next (e))
	    {
	      struct index_entry *en = list_entry (e, struct index_entry, elem);	
	      if ((en->status & S_IPST) &&
		  !(en->status & S_IADD) && 
		  (en->status & S_IDRT) && 
		  (en->status & S_ITRK)) 
		{
		  npath = (char *) malloc (strlen (path) +
					   strlen (en->name) + 2);
		  if (path[strlen (path) -1] == '/')
		    sprintf (npath, "%s%s", path, en->name);
		  else
		    sprintf (npath, "%s/%s", path, en->name);
		  
		  if (lstat64 (npath, &st_buf) != 0) fail("%s does not exist", npath);
		  if (st_buf.st_mode & S_IFDIR) {
		    if (commit_branch_using_tree (&ur, &tree, en->name) != 0)
		      fail ("%s commit failed", npath);		  
		  }
		  if (st_buf.st_mode & S_IFREG) {
		    if (commit_blob_using_tree (&ur, &tree, en->name, &ptree, msg) != 0)
		      fail ("%s commit failed", npath);
		  }
		  
		  status = index_entry_get_status (&index, en->name);
		  status &= ~S_IADD;
		  index_entry_set_status (&index, en->name, status);
		  
		  printf ("commit: %s\n", npath);
		  free (npath);
		}
	    }
	}
      
      // we're ready to objectify and commit the new tree
      if (tree_objectify (&ur, &tree, sha1) != 0)
	fail ("fail to write new tree for %s", path);
      
      if (commit_create (&commit, psha1, null_sha1, TREE_TYPE, sha1, msg) != 0)
	fail ("fail to generate new commit for %s", path);
      
      if (commit_objectify (&ur, &commit, sha1) != 0)
	fail ("fail to write commit for %s", path);

      npath = (char *) malloc (strlen (UR_DIR_HEADS) +
			       strlen (branchname) + 2);
      sprintf (npath, "%s/%s", UR_DIR_HEADS, branchname);
      
      if ((fd = file_open (path, npath, O_WRONLY | O_TRUNC | O_CREAT)) < 0)
	fail ("fail to open branch %s", branchname);
      sha1_to_hex (sha1, buf);
      writeline (fd, buf, 40, "\n");
      close (fd);

      // finally we update the index
      if (index_update (&ur, &index) != 0)
	fail ("fail to update index for %s", ur.path);
      if (index_write (&ur, &index) != 0)
	fail ("fail to write index for %s", ur.path);


      free (npath); npath = NULL;
      free (branchname); branchname = NULL;
      commit_destroy (&commit);
      tree_destroy (&tree);
      index_destroy (&index);
      state_destroy (&ur);    
    }
  else
    return 0;  

  return 0;
}
