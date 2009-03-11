#ifndef _OBJECT_H
#define _OBJECT_H

#define TYPE_BITS 3

struct object_list {
  struct object *item;
  struct object_list *next;
};

struct object {
  unsigned type : TYPE_BITS;
  size_t size;
  unsigned char sha1[20];
};


#endif
