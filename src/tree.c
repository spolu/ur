#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"
#include "list.h"
#include "io.h"
#include "sha1.h"

struct tree TREE_INITIALIZER;

int
init_tree ()
{
  memset (&TREE_INITIALIZER, 0, sizeof (struct tree));
  return 0;
}


int 
tree_objectify (state_t *ur, struct tree *tree, unsigned char sha1[20])
{
  struct list_elem *e;
  int od = -1;
  char buf[50];

  if ((od = object_create (ur)) < 0)
    goto error;
  
  sprintf (buf, "%d", (int) list_size (&tree->blob_entries));
  writeline (od, buf, strlen (buf), "\n");
  
  for (e = list_begin (&tree->blob_entries); e != list_end (&tree->blob_entries);
       e = list_next (e))
    {
      struct blob_tree_entry *en = list_entry (e, struct blob_tree_entry, elem);
      writeline (od, en->name, strlen (en->name), "\n");
      sha1_to_hex (en->commit, buf);
      writeline (od, buf, strlen (buf), "\n");
    }
  
  sprintf (buf, "%d", (int) list_size (&tree->branch_entries));
  writeline (od, buf, strlen (buf), "\n");
  
  for (e = list_begin (&tree->branch_entries); e != list_end (&tree->branch_entries);
       e = list_next (e))
    {
      struct branch_tree_entry *en = list_entry (e, struct branch_tree_entry, elem);
      writeline (od, en->name, strlen (en->name), "\n");
      writeline (od, en->branch, strlen (en->branch), "\n");
      sha1_to_hex (en->commit, buf);
      writeline (od, buf, strlen (buf), "\n");
    }  
  
  return object_finalize (ur, od, sha1);

 error:
  return -1;
}


int 
tree_read (state_t *ur, struct tree *tree, unsigned char sha1[20])
{
  int fd = -1, i;
  int blob_cnt = 0;
  int branch_cnt = 0;
  char * buf = NULL;
  char *name = NULL, *branch = NULL;
  unsigned char commit[20];
  struct blob_tree_entry *blob_entry = NULL;
  struct branch_tree_entry *branch_entry = NULL;

  tree_init (tree);

  if ((fd = object_open (ur, sha1)) < -1)
    goto error;
  
  buf = readline (fd);
  if (buf == NULL) goto error;
  blob_cnt = atoi (buf);
  free (buf); buf = NULL;

  for (i = 0; i < blob_cnt; i ++) 
    {        
      buf = readline (fd);
      if (buf == NULL) goto error;
      name = buf;
      buf = NULL;

      buf = readline (fd);
      if (buf == NULL) goto error;
      hex_to_sha1 (buf, commit);
      free (buf); buf = NULL;
      
      blob_entry = (struct blob_tree_entry *) malloc (sizeof (struct blob_tree_entry));
      if (blob_entry == NULL)
	goto error;
      
      blob_entry->name = name;
      memcpy (blob_entry->commit, commit, 20);

      list_push_back (&tree->blob_entries, &blob_entry->elem);

      name = NULL;
      blob_entry = NULL;
  }
  

  buf = readline (fd);
  if (buf == NULL) goto error;
  branch_cnt = atoi (buf);
  free (buf); buf = NULL;

  for (i = 0; i < branch_cnt; i ++) 
    {        
      buf = readline (fd);
      if (buf == NULL) goto error;
      name = buf;
      buf = NULL;

      buf = readline (fd);
      if (buf == NULL) goto error;
      branch = buf;
      buf = NULL;

      buf = readline (fd);
      if (buf == NULL) goto error;
      hex_to_sha1 (buf, commit);
      free (buf); buf = NULL;
      
      branch_entry = (struct branch_tree_entry *) malloc (sizeof (struct branch_tree_entry));
      if (branch_entry == NULL)
	goto error;
      
      branch_entry->name = name;
      branch_entry->branch = branch;
      memcpy (branch_entry->commit, commit, 20);

      list_push_back (&tree->branch_entries, &branch_entry->elem);

      name = NULL;
      branch = NULL;
      branch_entry = NULL;
  }

  close (fd);
  
  return 0;

 error:
  if (name != NULL) free (name);
  if (branch != NULL) free (branch);
  if (blob_entry != NULL) free (blob_entry);
  if (branch_entry != NULL) free (branch_entry);
  
  tree_destroy (tree);
  
  return -1;
}


int 
tree_init (struct tree *tree)
{
  tree->alive = false;
  
  list_init (&tree->blob_entries);
  list_init (&tree->branch_entries);

  tree->alive = true;

  return 0;
}

int 
tree_destroy (struct tree *tree)
{
  if (tree->alive)
    {
      
      while (list_size (&tree->blob_entries) > 0)
	{
	  struct list_elem *e = list_front (&tree->blob_entries);
	  list_remove (e);
	  struct blob_tree_entry *en = list_entry (e, struct blob_tree_entry, elem);
	  free (en->name);
	  free (en->commit);
	  free (en);
	}
      
      
      while (list_size (&tree->branch_entries) > 0)
	{
	  struct list_elem *e = list_front (&tree->branch_entries);
	  list_remove (e);
	  struct branch_tree_entry *en = list_entry (e, struct branch_tree_entry, elem);
	  free (en->name);
	  free (en->branch);
	  free (en);
	}
    }

  tree->alive = false;
  return 0;
}


int 
tree_blob_entry_add (struct tree *tree, char *name, unsigned char commit[20])
{
  struct blob_tree_entry *entry = NULL;

  tree_entry_remove (tree, name);
  
  entry = (struct blob_tree_entry *) malloc (sizeof (struct blob_tree_entry));
  if (entry == NULL)
    goto error;

  entry->name = NULL;

  entry->name = (char *) malloc (strlen (name) + 1);
  if (entry->name == NULL) goto error;

  strcpy (entry->name, name);
  memcpy (entry->commit, commit, 20);

  list_push_back (&tree->blob_entries, &entry->elem);

  return 0;

 error:
  if (entry != NULL) {
    if (entry->name != NULL) free (entry->name);
    free (entry);
  }  

  return -1;
}


int 
tree_branch_entry_add (struct tree *tree, char *name, char *branch, unsigned char commit[20])
{
  struct branch_tree_entry *entry = NULL;

  tree_entry_remove (tree, name);
  
  entry = (struct branch_tree_entry *) malloc (sizeof (struct branch_tree_entry));
  if (entry == NULL) goto error;

  entry->name = NULL;
  entry->branch = NULL;

  entry->name = (char *) malloc (strlen (name) + 1);
  if (entry->name == NULL) goto error;
  entry->branch = (char *) malloc (strlen (branch) + 1);
  if (entry->branch == NULL) goto error;

  strncpy (entry->name, name, strlen (name));
  strncpy (entry->branch, branch, strlen (branch));
  memcpy (entry->commit, commit, 20);

  list_push_back (&tree->branch_entries, &entry->elem);

  return 0;

 error:
  if (entry != NULL) {
    if (entry->name != NULL) free (entry->name);
    if (entry->branch != NULL) free (entry->branch);
    free (entry);
  }  

  return -1;
}


int
tree_entry_remove (struct tree *tree, char *name)
{
  struct blob_tree_entry *blob_entry = NULL;
  struct branch_tree_entry *branch_entry = NULL;
  struct list_elem *e;
  
  for (e = list_begin (&tree->blob_entries); e != list_end (&tree->blob_entries);
       e = list_next (e))
    {
      struct blob_tree_entry *en = list_entry (e, struct blob_tree_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	blob_entry = en;
      }
    }

  for (e = list_begin (&tree->branch_entries); e != list_end (&tree->branch_entries);
       e = list_next (e))
    {
      struct branch_tree_entry *en = list_entry (e, struct branch_tree_entry, elem);
      if (strncmp (en->name, name, strlen (name)) == 0) {
	branch_entry = en;
      }
    }

  if (blob_entry != NULL) {
    list_remove (&blob_entry->elem);
    free (blob_entry->name);
    free (blob_entry);    
  }

  if (branch_entry != NULL) {
    list_remove (&branch_entry->elem);
    free (branch_entry->name);
    free (branch_entry->branch);
    free (branch_entry);    
  }

  return 0;
}
