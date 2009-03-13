#ifndef _UR_OBJECT_H
#define _UR_OBJECT_H

#include "list.h"


/*
 * returns an object descriptor at the begining of object.
 */
int object_open (const unsigned char sha1[20]);

/*
 * returns a file descriptor for writing.
 */
int object_create ();

/*
 * finalizes object from file descriptor.
 */
int object_finalize (int fd, unsigned char sha1[20]);

/*
 * returns the size of an object.
 */
size_t object_size (unsigned char sha1[20]);


#endif
