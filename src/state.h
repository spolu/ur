#ifndef _UR_STATE_H
#define _UR_STATE_H

#define UR_DIR ".ur"
#define UR_DIR_OBJECT ".ur/objects"
#define UR_DIR_HEADS ".ur/heads"
#define UR_DIR_SHADOWS ".ur/shadows"
#define UR_INDEX ".ur/index"
#define UR_LOCK ".ur/lock"

#include <stdbool.h>
#include "list.h"


struct index_entry
{
  const char *name;
  bool dirty;
  bool added;
  struct timespec ctime;   //checkout time

  struct list_elem elem;
};

struct state 
{
  const char *path;

  char **branches;
  char *branch;
  struct tree *head;

  struct list index;
};


/*
 * global ur state used by all functions to perform action on the
 * curently considered directory.
 */
struct state ur_state;


/*
 * checks if the directory is tracked by looking a the .ur directory
 * and checking that there is a valid branch and underlying tree
 * object.
 */
int state_check (const char *path);

/*
 * init the state for a directory creating a .ur subdirectory if it
 * does not exist.
 */
int state_init (const char *path);

/*
 * read the state for a directory.
 */
int state_read (const char *path);

/*
 * locks and unlocks a .ur directory.
 */
int ur_lock (const char *path);
int ur_unlock (const char *path);


#endif
