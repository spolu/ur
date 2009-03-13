#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "object.h"
#include "state.h"
#include "list.h"
#include "sha1.h"
#include "io.h"

static int obj_nb;
static bool initialized = false;

struct obj_fd_entry {
  int fd;
  int objd;
  struct list_elem elem;
};

struct list obj_fd_map;

int 
object_open (const unsigned char sha1[20])
{
  char *path = NULL;
  char hex[50];
  int fd = -1;

  path = (char *) malloc (strlen (UR_DIR_OBJECTS) + 40 + 2);
  if (path == NULL)
    goto error;
  
  sha1_to_hex (sha1, hex);  
  sprintf (path, "%s/%s", UR_DIR_OBJECTS, hex);
  
  if ((fd = file_open (ur_state.path, path, O_RDONLY)) < 0)
    goto error;
  
  return fd;

 error:
  if (path != NULL)
    free (path);
  return -1;
}

int 
object_create ()
{
  char *tmp_path = NULL;
  int fd = -1;
  int objd;

  if (!initialized) {
    list_init (&obj_fd_map);
    obj_nb = 0;
    initialized = true;
  }

  objd = ++obj_nb;

  tmp_path = (char *) malloc (strlen (UR_DIR) + strlen ("obj_tmp-") + 40);
  if (tmp_path == NULL)
    goto error;
  
  sprintf (tmp_path, "%s/obj_tmp-%d", UR_DIR, objd);
  
  if ((fd = file_open (ur_state.path, tmp_path, O_CREAT | O_TRUNC | O_WRONLY)) < 0)
    goto error;

  fchmod (fd, S_IRUSR | S_IWUSR | S_IROTH);

  free (tmp_path);
  tmp_path = NULL;
  
  struct obj_fd_entry * entry = (struct obj_fd_entry *) malloc (sizeof (struct obj_fd_entry));
  entry->fd = fd;
  entry->objd = objd;

  list_push_back (&obj_fd_map, &entry->elem);

  return fd;

 error:
  if (tmp_path != NULL)
    free (tmp_path);
  return -1;
}

int 
object_finalize (int fd, unsigned char sha1[20])
{
  char *tmp_path = NULL;
  char *obj_path = NULL;
  struct list_elem *e;
  struct obj_fd_entry *entry = NULL;
  ur_SHA_CTX ctx;

  char hex[50];

  char buf[512];
  size_t len;

  if (!initialized) {
    list_init (&obj_fd_map);
    obj_nb = 0;
    initialized = true;
  }

  close (fd);
  
  for (e = list_begin (&obj_fd_map); e != list_end (&obj_fd_map);
       e = list_next (e))
    {
      struct obj_fd_entry *en = list_entry (e, struct obj_fd_entry, elem);
      if (en->fd == fd) {
	entry = en;
	break;
      }
    }
  
  if (entry == NULL)
    goto error;

  list_remove (&entry->elem);
  
  tmp_path = (char *) malloc (strlen (ur_state.path) + 
			      strlen (UR_DIR) + 
			      strlen ("obj_tmp-") + 40);
  if (tmp_path == NULL)
    goto error;
  
  sprintf (tmp_path, "%s/%s/obj_tmp-%d", ur_state.path, UR_DIR, entry->objd);
  
  free (entry);
  entry = NULL;

  if ((fd = open (tmp_path, O_RDONLY)) < 0)
    goto error;
  
  ur_SHA1_Init (&ctx);
  
  while ((len = readn (fd, buf, 512)) > 0) {
    ur_SHA1_Update (&ctx, buf, len);
  }

  close (fd);

  ur_SHA1_Final (sha1, &ctx);
  
  obj_path = (char *) malloc (strlen (ur_state.path) +
			      strlen (UR_DIR_OBJECTS) + 50);
  if (obj_path == NULL)
    goto error;

  sha1_to_hex (sha1, hex);
  sprintf (obj_path, "%s/%s/%s", ur_state.path, UR_DIR_OBJECTS, hex);
  
  if (link (tmp_path, obj_path) != 0)
    goto error;

  if (unlink (tmp_path) != 0)
    goto error;

  free (tmp_path);
  free (obj_path);

  return 0;

 error:
  if (tmp_path != NULL)
    free (tmp_path);
  if (entry != NULL)
    free (entry);
  if (obj_path != NULL)
    free (obj_path);
  return -1;
}

size_t 
object_size (unsigned char sha1[20])
{
  return 0;
}
