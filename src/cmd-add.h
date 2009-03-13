#ifndef _UR_CMD_ADD_H
#define _UR_CMD_ADD_H

#include "ur.h"

int cmd_add (char *path, bool recursive);

int dir_add (char *path);
int dir_init (char *path);
int file_add (char *path);

#endif
