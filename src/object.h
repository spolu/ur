#ifndef _UR_OBJECT_H
#define _UR_OBJECT_H

#include "list.h"

struct object 
{
  unsigned type : TYPE_BITS;
  size_t size;
  unsigned char sha1[20];

  struct list_elem elem;
};

#endif
