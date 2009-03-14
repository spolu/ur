#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "index.h"
#include "ur.h"
#include "io.h"
#include "sha1.h"
#include "helper.h"


struct index INDEX_INITIALIZER;

int
init_index ()
{
  memset (&INDEX_INITIALIZER, 0, sizeof (struct index));
  return 0;
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
      cp = strptime (buf, "%a %b %d %T %Y", &tm);
      ctime = mktime (&tm);
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
	ct = ctime (&en->ctime);
	writeline (fd, ct, strlen (ct) - 1, "\n");      	
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

  strncpy (entry->name, name, strlen (name));
  entry->status = status;
  entry->ctime = ctime;

  ASSERT (entry->status & S_IPST);
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
    ASSERT (entry->status & S_IPST);
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
    ASSERT (entry->status & S_IPST);
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
    ASSERT (entry->status & S_IPST);
    ASSERT (!(entry->status & S_ITRK) || entry->ctime > 0);
    ASSERT (!entry->ctime > 0 || entry->status & S_ITRK);
    index->dirty = true;
    return 0;
  }

  ASSERT (status & S_IPST);
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

  return index_entry_set (index, name, S_IPST | S_ITRK, ctime);

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

  return 0;
}
