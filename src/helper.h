#ifndef _UR_HELPER_H
#define _UR_HELPER_H

/*
 * helper functions for .ur subdir access
 */
int subdir_check (const char *root, const char *path);
int file_check (const char *root, const char *path);
int subdir_create (const char *root, const char *path);
int file_open (const char *root, const char *path, int oflag);


/*
 * parent_dir / filename extraction
 */
int parent_dir (const char *path, char * parent);
int filename (const char *path, char *name);


#endif
