#ifndef _UR_OBJECT_H
#define _UR_OBJECT_H

#include "list.h"


/*
 * returns an object descriptor at the begining of object.
 */
int object_open (unsigned char sha1[20]);

/*
 * close the the object opened for read.
 */
int object_close (int od);

/*
 * returns an object descriptor for writing.
 */
int object_create ();

/*
 * finalizes object from descriptor.
 */
int object_finalize (int od);

/*
 * returns the size of an object.
 */
size_t object_size (unsigned char sha1[20]);


#endif
