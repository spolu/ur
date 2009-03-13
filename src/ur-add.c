#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd-add.h"

int
main (int argc, char ** argv)
{
  if (argc != 2 && argc != 3) {
    printf ("usage: ur-add [-r] path\n");
    return -1;
  }

  if (argc == 3) {
    if (strcmp (argv[1], "-r") != 0) {
      printf ("unknow option: %s\n", argv[1]);
      return -1;
    }
    return cmd_add (argv[2], true);
  }
  
  if (argc == 2) {
    return cmd_add (argv[1], false);    
  }
  
  return 0;
}
