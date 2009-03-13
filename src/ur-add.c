#include <stdlib.h>
#include <stdio.h>

#include "cmd-add.h"

int
main (int argc, char ** argv)
{
  if (argc != 2 || argc != 3) {
    printf ("usage:\nur-add [-r] path\n");
    return -1;
  }
  
  return dir_init (".");
}
