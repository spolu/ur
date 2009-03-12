#ifndef _UR_BLOB_H
#define _UR_BLOB_H


/*
 * Creates a new object for blob designed by a the file descriptor.
 */
int blob_objectify (int fd, unsigned char *sha1[20]);

#endif
