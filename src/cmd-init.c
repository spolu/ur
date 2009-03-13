#include <stdlib.h>
#include <stdio.h>

#include "cmd-init.h"


int 
cmd_init (char *path)
{
  if (state_check (".") == 0) {
    printf ("ur directory already initialized\n");
    return -1;
  }
  
  if (state_init (".") != 0) {
    printf ("ur directory initialization failed\n");
    return -1;
  }
  
  printf ("ur directory initialized\n");
  
  return 0;
}
