#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tree.h"
#include "list.h"
#include "io.h"

struct tree TREE_INITIALIZER;

int
init_tree ()
{
  memset (&TREE_INITIALIZER, 0, sizeof (struct tree));
  return 0;
}


int 
tree_objectify (struct tree *tree, unsigned char sha1[20])
{
  struct list_elem *e;
  int od = -1;
  char buf[50];

  if ((od = object_create ()) < 0)
    goto error;
  
  sprintf (buf, "%d", (int) list_size (&tree->blob_entries));
  writeline (od, buf, strlen (buf), "\n");
  
  for (e = list_begin (&tree->blob_entries); e != list_end (&tree->blob_entries);
       e = list_next (e))
    {
      struct blob_tree_entry *en = list_entry (e, struct blob_tree_entry, elem);
      writeline (od, en->name, strlen (en->name), "\n");
      writeline (od, en->commit, strlen (en->commit), "\n");
    }
  
  sprintf (buf, "%d", (int) list_size (&tree->branch_entries));
  writeline (od, buf, strlen (buf), "\n");
  
  for (e = list_begin (&tree->branch_entries); e != list_end (&tree->branch_entries);
       e = list_next (e))
    {
      struct branch_tree_entry *en = list_entry (e, struct branch_tree_entry, elem);
      writeline (od, en->name, strlen (en->name), "\n");
      writeline (od, en->branch, strlen (en->branch), "\n");
    }  
  
  return object_finalize (od, sha1);

 error:
  return -1;
}


int 
tree_read (struct tree *tree, unsigned char sha1[20])
{
  int fd = -1, i;
  int blob_cnt = 0;
  int branch_cnt = 0;
  char * buf;
  char *name = NULL, *commit = NULL, *branch = NULL;
  struct blob_tree_entry *blob_entry = NULL;
  struct branch_tree_entry *branch_entry = NULL;

  tree_init (tree);

  if ((fd = object_open (sha1)) < -1)
    goto error;
  
  buf = NULL;
  buf = readline (fd);
  if (buf == NULL)
    goto error;
  blob_cnt = atoi (buf);
  free (buf);

  for (i = 0; i < blob_cnt; i ++) 
    {        
      name = NULL;
      commit = NULL;
      blob_entry = NULL;

      buf = NULL;
      buf = readline (fd);
      if (buf == NULL)
	goto error;
      name = buf;

      buf = NULL;
      buf = readline (fd);
      if (buf == NULL)
	goto error;
      commit = buf;
      
      blob_entry = (struct blob_tree_entry *) malloc (sizeof (struct blob_tree_entry));
      if (blob_entry == NULL)
	goto error;
      
      blob_entry->name = name;
      blob_entry->commit = commit;

      list_push_back (&tree->blob_entries, &blob_entry->elem);
  }
  

  buf = NULL;
  buf = readline (fd);
  if (buf == NULL)
    goto error;
  branch_cnt = atoi (buf);
  free (buf);

  for (i = 0; i < branch_cnt; i ++) 
    {        
      name = NULL;
      branch = NULL;
      branch_entry = NULL;

      buf = NULL;
      buf = readline (fd);
      if (buf == NULL)
	goto error;
      name = buf;

      buf = NULL;
      buf = readline (fd);
      if (buf == NULL)
	goto error;
      branch = buf;
      
      branch_entry = (struct branch_tree_entry *) malloc (sizeof (struct branch_tree_entry));
      if (branch_entry == NULL)
	goto error;
      
      branch_entry->name = name;
      branch_entry->branch = branch;

      list_push_back (&tree->branch_entries, &branch_entry->elem);
  }

  close (fd);
  
  return 0;
 error:
  if (name != NULL)
    free (name);
  if (commit != NULL)
    free (commit);
  if (branch != NULL)
    free (branch);
  if (blob_entry != NULL)
    free (blob_entry);
  if (branch_entry != NULL)
    free (branch_entry);
  
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
tree_blob_entry_add (struct tree *tree, char *name, char *commit)
{
  struct blob_tree_entry *entry = NULL;

  tree_remove_entry (tree, name);
  
  entry = (struct blob_tree_entry *) malloc (sizeof (struct blob_tree_entry));
  if (entry == NULL)
    goto error;

  entry->name = NULL;
  entry->commit = NULL;

  entry->name = (char *) malloc (strlen (name) + 1);
  if (entry->name == NULL)
    goto error;
  entry->commit = (char *) malloc (strlen (commit) + 1);
  if (entry->commit == NULL)
    goto error;

  strcpy (entry->name, name);
  strcpy (entry->commit, commit);

  list_push_back (&tree->blob_entries, &entry->elem);

  return 0;

 error:
  if (entry != NULL) {
    if (entry->name != NULL)
      free (entry->name);
    if (entry->commit != NULL)
      free (entry->commit);
    free (entry);
  }  
  return -1;
}


int 
tree_branch_entry_add (struct tree *tree, char *name, char *branch)
{
  struct branch_tree_entry *entry = NULL;

  tree_remove_entry (tree, name);
  
  entry = (struct branch_tree_entry *) malloc (sizeof (struct branch_tree_entry));
  if (entry == NULL)
    goto error;

  entry->name = NULL;
  entry->branch = NULL;

  entry->name = (char *) malloc (strlen (name) + 1);
  if (entry->name == NULL)
    goto error;
  entry->branch = (char *) malloc (strlen (branch) + 1);
  if (entry->branch == NULL)
    goto error;

  strncpy (entry->name, name, strlen (name));
  strncpy (entry->branch, branch, strlen (branch));

  list_push_back (&tree->branch_entries, &entry->elem);

  return 0;

 error:
  if (entry != NULL) {
    if (entry->name != NULL)
      free (entry->name);
    if (entry->branch != NULL)
      free (entry->branch);
    free (entry);
  }  
  return -1;
}


int
tree_remove_entry (struct tree *tree, char *name)
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
    free (blob_entry->commit);
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
