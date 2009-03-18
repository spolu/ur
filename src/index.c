#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>


#include "debug.h"
#include "index.h"
#include "ur.h"
#include "io.h"
#include "sha1.h"
#include "helper.h"
#include "branch.h"
#include "commit.h"



struct index INDEX_INITIALIZER;

static int clear_present (struct index *index);

int
init_index ()
{
  memset (&INDEX_INITIALIZER, 0, sizeof (struct index));
  return 0;
}

int 
index_update (state_t *ur, struct index *index)
{
  char *branchname = NULL, *npath = NULL, *buf = NULL;
  struct tree tree = TREE_INITIALIZER;
  struct commit commit = COMMIT_INITIALIZER;
  struct list_elem *e;
  unsigned char sha1[20], nsha1[20];
  int status, fd = -1;
  time_t ctime;
  struct stat64 st_buf;
  DIR *dp;
  struct dirent *ep;
  struct blob_tree_entry blob;
  struct branch_tree_entry branch;
  ur_SHA_CTX ctx;
  size_t len;

  if (!index->alive) goto error;
  
  branchname = branch_get_head_name (ur);
  if (branchname == NULL) goto error;
  if (branch_read_tree (ur, &tree, branchname) != 0) goto error;
  free (branchname); branchname = NULL;

  for (e = list_begin (&tree.blob_entries); e != list_end (&tree.blob_entries);
       e = list_next (e))
    {
      struct blob_tree_entry *en = list_entry (e, struct blob_tree_entry, elem);

      status = index_entry_get_status (index, en->name);
      status |= S_ITRK;
      
      memcpy (sha1, en->commit, 20);      
      if (commit_read (ur, &commit, sha1) != 0) goto error;
      ctime = commit.ctime;
      commit_destroy (&commit);
      
      if (index_entry_set (index, en->name, status, ctime) != 0) goto error;      
    }
  
  for (e = list_begin (&tree.branch_entries); e != list_end (&tree.branch_entries);
       e = list_next (e))
    {
      struct branch_tree_entry *en = list_entry (e, struct branch_tree_entry, elem);

      status = index_entry_get_status (index, en->name);
      status |= S_ITRK;
      
      memcpy (sha1, en->commit, 20);      
      if (commit_read (ur, &commit, sha1) != 0) goto error;
      ctime = commit.ctime;
      commit_destroy (&commit);
      
      if (index_entry_set (index, en->name, status, ctime) != 0) goto error;      
    }  
    
  clear_present (index);

  if ((dp = opendir (ur->path)) == NULL) goto error;

  while ((ep = readdir (dp))) {
    if (ep->d_name[0] != '.') 
      {		
	npath = (char *) malloc (strlen (ur->path) +
				 strlen (ep->d_name) + 2);
	if (npath == NULL) goto error;

	if (ur->path[strlen (ur->path) - 1] == '/')
	  sprintf (npath, "%s%s", ur->path, ep->d_name);
	else
	  sprintf (npath, "%s/%s", ur->path, ep->d_name);
	
	if (lstat64 (npath, &st_buf) != 0) goto error;

	if (st_buf.st_mode & S_IFREG)
	  {
	    status = index_entry_get_status (index, ep->d_name);
	    status |= S_IPST;

	    /*
	      if (index_entry_get_ctime (index, ep->d_name) < st_buf.st_mtimespec.tv_sec ||
	      index_entry_get_ctime (index, ep->d_name) < st_buf.st_ctimespec.tv_sec)
	      {
	    */ //Optimization
		
	    if(tree_get_blob_entry (&tree, ep->d_name, &blob) == 0)
	      {
		printf ("COMPUTING SHA1\n");
		memcpy (sha1, blob.commit, 20);
		if (commit_read (ur, &commit, sha1) != 0) goto error;
		memcpy (sha1, commit.object_sha1, 20);
		commit_destroy (&commit);
		
		if ((fd = open (npath, O_RDONLY)) < 0)
		  goto error;		
		ur_SHA1_Init (&ctx);		
		while ((len = readn (fd, buf, 512)) > 0) {
		  ur_SHA1_Update (&ctx, buf, len);
		}		
		close (fd);		
		ur_SHA1_Final (nsha1, &ctx);
		
		if (memcmp (nsha1, sha1, 20) != 0) {
		  status |= S_IDRT;
		}		
	      }
	    else {
	      status &= ~S_ITRK;
	      status |= S_IDRT;
	    }
	    
	    index_entry_set_status (index, ep->d_name, status);
	  }
	
	if (st_buf.st_mode & S_IFDIR)
	  {
	    status = index_entry_get_status (index, ep->d_name);
	    status |= S_IPST;	    	    	    
	    	    
	    if(tree_get_branch_entry (&tree, ep->d_name, &branch) == 0)
	      {
		state_t nur = STATE_INITIALIZER;

		if (ur_check (npath) != 0) {
		  status |= S_IDRT;
		  status &= ~S_ITRK;
		}
		
		else if (state_init (&nur, npath) == 0) 
		  {
		    branchname = branch_get_head_name (ur);
		    //printf ("branchname : %s\n", branchname);
		    if (branchname == NULL) goto error;
		    if (branch_read_commit_sha1(&nur, sha1, branchname) != 0) goto error;
		    
		    if (strcmp (branch.branch, branchname) != 0 ||
			memcmp (branch.commit, sha1, 20) != 0) {
		      status |= S_IDRT;      
		    }
		    free (branchname); branchname = NULL;		    
		}
	      }	    
	    else {
	      status &= ~S_ITRK;
	      status |= S_IDRT;
	    }
	    
	    index_entry_set_status (index, ep->d_name, status);
	  }

	free (npath); npath = NULL;
      }	    
  }
  (void) closedir (dp);
    
  index_write (ur, index);
  tree_destroy (&tree);

  return 0;

 error:
  tree_destroy (&tree);
  commit_destroy (&commit);
  if (branchname != NULL) free (branchname);
  if (npath != NULL) free (npath);

  return -1;
}


int
index_read (state_t *ur, struct index *index)
{
  char * buf = NULL, *name = NULL, *cp = NULL;
  int status = 0, cnt, i, fd = -1;
  time_t ctime;
  struct tm tm;
  struct index_entry *index_entry = NULL;

  index_destroy (index);
  list_init (&index->entries);

  if ((fd = file_open (ur->path, UR_INDEX, O_RDONLY)) < 0)
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
      status = atoi (buf);
      free (buf); buf = NULL;
            
      buf = readline (fd);  
      if (buf == NULL) goto error;
      if (strncmp (buf, "UNSET", strlen ("UNSET")) == 0) {
	ctime = 0;
      }
      else {
	cp = strptime (buf, "%a %b %d %T %Y", &tm);
	ctime = mktime (&tm);
      }
      free (buf); buf = NULL;  
    
      index_entry = (struct index_entry *) malloc (sizeof (struct index_entry));
      if (index_entry == NULL)
	goto error;

      index_entry->name = name;
      index_entry->status = status;
      index_entry->ctime = ctime;

      list_push_back (&index->entries, &index_entry->elem);

      name = NULL;
      index_entry = NULL;
    }

  close (fd);
  index->alive = true;
  index->dirty = false;

  return 0;

 error:
  if (buf != NULL) free (buf);
  if (name != NULL) free (name);
  if (index_entry != NULL) free (index_entry);

  index->alive = true;
  index_destroy (index);

  return -1;
}


int
index_write (state_t *ur, struct index *index)
{
  struct list_elem *e;
  char buf[50];
  int fd = -1;
  char * ct;

  if (!index->alive) goto error;
  if (!index->dirty) return 0;
  
  if ((fd = file_open (ur->path, UR_INDEX, O_TRUNC | O_WRONLY)) < 0)
    goto error;
  
  sprintf (buf, "%d", (int) list_size (&index->entries));
  writeline (fd, buf, strlen (buf), "\n");

    for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
      {
	struct index_entry *en = list_entry (e, struct index_entry, elem);	

	writeline (fd, en->name, strlen (en->name), "\n");
	sprintf (buf, "%d", en->status);
	writeline (fd, buf, strlen (buf), "\n");	
	if (en->ctime == 0) {
	  sprintf (buf, "UNSET");
	  writeline (fd, buf, strlen (buf), "\n");
	}
	else { 
	  ct = ctime (&en->ctime);
	  writeline (fd, ct, strlen (ct) - 1, "\n");
	}
      }

    index->dirty = false;
    return 0;

 error:
  return -1;
}

int 
index_entry_set (struct index *index, char *name, int status, time_t ctime)
{
  struct index_entry *entry = NULL;

  if (!index->alive) goto error;

  index_entry_remove (index, name);

  entry = (struct index_entry *) malloc (sizeof (struct index_entry));
  if (entry == NULL) goto error;

  entry->name = NULL;
  entry->name = (char *) malloc (strlen (name) + 1);
  if (entry->name == NULL) goto error;

  strncpy (entry->name, name, strlen (name) + 1);
  entry->status = status;
  entry->ctime = ctime;

  ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
  ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);

  list_push_back (&index->entries, &entry->elem);
  index->dirty = true;

  return 0;

 error:
  if (entry != NULL) {
    if (entry->name != NULL) free (entry->name);
    free (entry);
  }
  
  return -1;
}


int 
index_entry_get_status (struct index *index, char *name)
{
  struct index_entry *entry = NULL;
  struct list_elem *e;

  if (!index->alive) goto error;

  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	entry = en;
      }
    }
  
  if (entry != NULL) {
    ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
    ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);
    return entry->status;
  }

  return 0;

 error:
  return -1;  
}


time_t 
index_entry_get_ctime (struct index *index, char *name)
{
  struct index_entry *entry = NULL;
  struct list_elem *e;

  if (!index->alive) goto error;

  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	entry = en;
      }
    }
  
  if (entry != NULL) {
    ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
    ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);
    return entry->ctime;
  }

  return 0;

 error:
  return -1;
}

int 
index_entry_set_status (struct index *index, char *name, int status)
{
  struct index_entry *entry = NULL;
  struct list_elem *e;

  if (!index->alive) goto error;
  
  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	entry = en;
      }
    }
  
  if (entry != NULL) {
    entry->status = status;
    ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
    ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);
    index->dirty = true;
    return 0;
  }

  ASSERT (!(status & S_ITRK));
  return index_entry_set (index, name, status, 0);

  return 0;

 error:
  return -1;  
}

int 
index_entry_set_ctime (struct index *index, char *name, time_t ctime)
{
  struct index_entry *entry = NULL;
  struct list_elem *e;

  if (!index->alive) goto error;

  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	entry = en;
      }
    }
  
  if (entry != NULL) {
    entry->ctime = ctime;
    ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
    ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);
    index->dirty = true;
    return 0;
  }

  return index_entry_set (index, name, S_ITRK, ctime);

 error:
  return -1;  
}



int index_entry_remove (struct index *index, char *name)
{
  struct index_entry *entry = NULL;
  struct list_elem *e;

  if (!index->alive) goto error;

  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	entry = en;
      }
    }
  
  if (entry != NULL) {
    list_remove (&entry->elem);
    free (entry->name);
    free (entry);    
    index->dirty = true;
  }

  return 0;

 error:
  return -1;
}


int
index_destroy (struct index *index)
{
  if (index->alive) 
    {
      ASSERT (!index->dirty);

      while (list_size (&index->entries) > 0)
	{
	  struct list_elem *e = list_front (&index->entries);
	  list_remove (e);
	  struct index_entry *en = list_entry (e, struct index_entry, elem);
	  free (en->name);
	  free (en);
	}
    }
  
  index->dirty = false;
  index->alive = false;
  *index = INDEX_INITIALIZER;

  return 0;
}


static int 
clear_present (struct index *index)
{
  struct list_elem *e;

  if (!index->alive) goto error;

  for (e = list_begin (&index->entries); e != list_end (&index->entries);
       e = list_next (e))
    {
      struct index_entry *en = list_entry (e, struct index_entry, elem);
      en->status &= ~S_IPST;
    }
  
  return 0;

 error:
  return -1;  
}
