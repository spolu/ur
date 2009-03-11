#ifndef _UR_COMMIT_H
#define _UR_COMMIT_H

#include "object.h"
#include "tree.h"
#include "list.h"


struct commit 
{
  struct object object;
  unsigned long date;
  struct list parents;    //commit list
  struct object *object;   //tree or blob
  char *buffer;

  struct list_elem elem;
};

#endif
