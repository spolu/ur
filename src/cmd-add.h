#ifndef _UR_CMD_ADD_H
#define _UR_CMD_ADD_H

#include "ur.h"

int cmd_add (const char *path, bool recursive);
int parent_dir (const char *path, char * parent);

#endif
