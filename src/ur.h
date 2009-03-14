#ifndef _UR_UR_H
#define _UR_UR_H

#define UR_DIR ".ur"
#define UR_DIR_OBJECTS ".ur/objects"
#define UR_DIR_HEADS ".ur/heads"
#define UR_DIR_SHADOWS ".ur/shadows"
#define UR_INDEX ".ur/index"
#define UR_LOCK ".ur/lock"
#define UR_HEAD ".ur/HEAD"

#define UR_MASTER "master"

typedef struct state 
{
  const char *path;
  bool alive;
} state_t;


/*
 * global ur state used by all functions to perform action on the
 * curently considered directory.
 */
extern state_t STATE_INITIALIZER;

/*
 * initialize ur modules
 */
int init_ur ();

/*
 * checks if the directory is tracked by looking a the .ur directory
 * and checking that there is a valid branch and underlying tree
 * object.
 */
int ur_check (const char *path);

/*
 * create a new ur directory creating a .ur subdirectory if it
 * does not exist at path.
 */
int ur_create (const char *path);


/*
 * read the state for a directory.
 */
int state_init (state_t *ur, const char *path);

/*
 * destroys the state data
 */
int state_destroy (state_t *ur);

/*
 * locks and unlocks a .ur directory.
 */
int ur_lock (state_t *ur);
int ur_unlock (state_t *ur);


#endif
