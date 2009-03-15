#ifndef _UR_CMD_H
#define _UR_CMD_H

#include <stdbool.h>

void fail (const char *fmt, ...);
void output (const char *fmt, ...);

int cmd_add (const char *path, bool recursive);
int cmd_status (const char *path, bool recursive);

#endif
