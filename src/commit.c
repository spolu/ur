#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "commit.h"
#include "object.h"
#include "io.h"
#include "sha1.h"

struct commit COMMIT_INITIALIZER;

int 
init_commit ()
{
  memset (&COMMIT_INITIALIZER, 0, sizeof (struct commit));
  return 0;
}


int 
commit_objectify (state_t *ur, struct commit *commit, unsigned char sha1[20])
{
  int od = -1;
  char * ct;
  char buf [50];

  if ((od = object_create (ur)) < 0) goto error;

  ct = ctime (&commit->date);
  writeline (od, ct, strlen (ct) - 1, "\n");
  sha1_to_hex (commit->parent_sha1_1, buf);
  writeline (od, buf, strlen (buf), "\n");
  sha1_to_hex (commit->parent_sha1_2, buf);
  writeline (od, buf, strlen (buf), "\n");
  sprintf (buf, "%d", commit->object_type);
  writeline (od, buf, strlen (buf), "\n");
  sha1_to_hex (commit->object_sha1, buf);
  writeline (od, buf, strlen (buf), "\n");
  sprintf (buf, "%d", (int) strlen (commit->msg));
  writeline (od, buf, strlen (buf), "\n");
  writen (od, commit->msg, strlen (commit->msg));

  return object_finalize (ur, od, sha1);

 error:
  return -1;
}

int 
commit_read (state_t *ur, struct commit *commit, const unsigned char sha1[20])
{
  int fd = -1;
  char * buf = NULL;
  int len;
  struct tm tm;
  char *cp;

  commit->msg = NULL;

  if (commit->alive) goto error;

  if ((fd = object_open (ur, sha1)) < -1) goto error;

  buf = readline (fd);  
  if (buf == NULL) goto error;
  cp = strptime (buf, "%a %b %d %T %Y", &tm);
  commit->date = mktime (&tm);
  free (buf); buf = NULL;

  buf = readline (fd);  
  if (buf == NULL) goto error;
  hex_to_sha1 (buf, commit->parent_sha1_1);
  free (buf); buf = NULL;

  buf = readline (fd);  
  if (buf == NULL) goto error;
  hex_to_sha1 (buf, commit->parent_sha1_2);
  free (buf); buf = NULL;

  buf = readline (fd);
  if (buf == NULL) goto error;
  commit->object_type = atoi (buf);
  free (buf); buf = NULL;

  buf = readline (fd);  
  if (buf == NULL) goto error;
  hex_to_sha1 (buf, commit->object_sha1);
  free (buf); buf = NULL;
  
  buf = readline (fd);
  if (buf == NULL) goto error;    
  len = atoi (buf);
  free (buf); buf = NULL;

  commit->msg = (char *) malloc (len);
  readn (fd, commit->msg, len);

  close (fd);

  commit->alive = true;

  return 0;

 error:  
 if (commit->msg != NULL) free (commit->msg);

  return -1;
}


int commit_create (struct commit *commit, 
		   const unsigned char parent_sha1_1[20],
		   const unsigned char parent_sha1_2[20],
		   const unsigned object_type,
		   const unsigned char object_sha1[20],
		   const char *msg)
{
  commit->alive = false;
  commit->msg = NULL;

  memcpy (commit->parent_sha1_1, parent_sha1_1, 20);
  memcpy (commit->parent_sha1_2, parent_sha1_2, 20);

  if (object_type != TREE_TYPE &&
      object_type != BLOB_TYPE)
    goto error;

  commit->object_type = object_type;
  memcpy (commit->object_sha1, object_sha1, 20);
  commit->msg = (char *) malloc (strlen (msg) + 1);
  
  if (commit->msg == NULL) goto error;
  
  strncpy (commit->msg, msg, strlen (msg) + 1);

  commit->date = time (NULL);  
  commit->alive = true;

  return 0;

 error:
  if (commit->msg != NULL) free (commit->msg);

  return -1;
}


int 
commit_destroy (struct commit *commit)
{
  if (commit->alive) {
    if (commit->msg != NULL) free (commit->msg);
  }
  commit->alive = false;
  
  return 0;
}

