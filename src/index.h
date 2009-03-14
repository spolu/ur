#ifndef _UR_INDEX_H
#define _UR_INDEX_H


#include <stdbool.h>
#include <time.h>

#include "list.h"
#include "ur.h"

struct state;

#define S_IPST 0000001
#define S_ITRK 0000010
#define S_IADD 0000100
#define S_IDRT 0001000

struct index_entry
{
  char *name;
  int status;
  time_t ctime;   //checkout time

  struct list_elem elem;
};


struct index
{
  struct list entries;

  bool dirty;
  bool alive;
};


extern struct index INDEX_INITIALIZER;

/*
 * tree module initialization
 */
int init_index ();

/*
 * reads the index from disk using ur_state path
 */
int index_read (state_t *ur, struct index *index);

/*
 * write back the index
 */
int index_write (state_t *ur, struct index *index);

/*
 * set an index entry in index. idempotent
 */
int index_entry_set (struct index *index, char *name, int status, time_t ctime);

/*
 * status operation function. status_set creates the entry if necessary.
 */
int index_entry_get_status (struct index *index, char *name);
time_t index_entry_get_ctime (struct index *index, char *name);

int index_entry_set_status (struct index *index, char *name, int status);
int index_entry_set_ctime (struct index *index, char *name, time_t ctime);

/*
 * removes entry associated with name in index.
 */
int index_entry_remove (struct index *index, char *name);

/*
 * cleans up the index in memory
 */
int index_destroy (struct index *index);

#endif
