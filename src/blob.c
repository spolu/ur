#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "object.h"
#include "io.h"
#include "blob.h"
#include "tree.h"
#include "commit.h"
#include "sha1.h"

int 
blob_objectify (state_t *ur, int fd, unsigned char sha1[20])
{
  int dest = -1;
  size_t len = 0;
  char buf[512];

  if ((dest = object_create (ur)) < 0) goto error;
  
  while ((len = readn (fd, buf, 512)) > 0) {
    writen (dest, buf, 512);
  }

  if (object_finalize (ur, dest, sha1) != 0) goto error;

  return 0;

 error:
  return -1;
}


int 
commit_blob_using_tree (state_t *ur, 
			struct tree *tree, 
			char *name,
			struct tree *ptree)
{
  struct blob_tree_entry blob;
  struct commit commit = COMMIT_INITIALIZER;
  ur_SHA_CTX ctx;
  unsigned char sha1[20];
  unsigned char nsha1[20];
  unsigned char commit_sha1[20];
  char *npath = NULL;
  char buf[512];

  npath = (char *) malloc (strlen (ur->path) +
			   strlen (ep->d_name) + 2);
  if (npath == NULL) goto error;
  
  if (ur->path[strlen (ur->path) - 1] == '/')
    sprintf (npath, "%s%s", ur->path, ep->d_name);
  else
    sprintf (npath, "%s/%s", ur->path, ep->d_name);
  
  if(tree_get_blob_entry (ptree, name, &blob) == 0)
    {
      printf ("COMPUTING SHA1\n");
      memcpy (commit_sha1, blob.commit, 20);
      if (commit_read (ur, &commit, commit_sha1) != 0) goto error;
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
      
      if (memcmp (nsha1, sha1, 20) != 0) 
	{
	  /*
	   * changed create new commit
	   */
	  if ((fd = open (npath, O_RDONLY)) < 0)
	    goto error;		
	  if (blob_objectify (fd, nsha1) != 0) goto error;
	  
	  if (commit_create (&commit, 
			     commit_sha1, null_sha1, 
			     BLOB_TYPE, nsha1, msg) != 0) goto error;
	  
	  if (commit_objectify (ur, &commit, commit_sha1) != 0) goto error;
	}	
      
      tree_blob_entry_add (tree, name, commit_sha1);
      commit_destroy (&commit);
    }
  
  else
    {
      if ((fd = open (npath, O_RDONLY)) < 0)
	goto error;		
      if (blob_objectify (fd, nsha1) != 0) goto error;
      
      if (commit_create (&commit, 
			 null_sha1, null_sha1, 
			 BLOB_TYPE, nsha1, msg) != 0) goto error;
      
      if (commit_objectify (ur, &commit, commit_sha1) != 0) goto error;
    }

  commit_destroy (&commit);
  free (npath); npath = NULL;
  
  return 0;
  
 error:
  if (npath != NULL) free (npath);
  commit_destroy (&commit);
  return -1;
}
