#ifndef _UR_OBJECT_H
#define _UR_OBJECT_H

#include "list.h"
#include "ur.h"

/*
 * returns an object descriptor at the begining of object.
 */
int object_open (state_t *ur, const unsigned char sha1[20]);

/*
 * returns a file descriptor for writing.
 */
int object_create (state_t *ur);

/*
 * finalizes object from file descriptor.
 */
int object_finalize (state_t *ur, int fd, unsigned char sha1[20]);

/*
 * returns the size of an object.
 */
size_t object_size (state_t *ur, unsigned char sha1[20]);


#endif
