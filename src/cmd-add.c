#include <stdlib.h>
#include <stdio.h>

#include "cmd-add.h"


int
cmd_add (char *path, bool recursive)
{
  
  return 0;
}

int 
dir_init (char *path)
{
  if (state_check (path) == 0) {
    printf ("%s already initialized\n", path);
    return -1;
  }
  
  if (state_init (path) != 0) {
    printf ("%s initialization failed\n", path);
    return -1;
  }
  
  printf ("%s initialized\n", path);
  
  return 0;
}

