#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ur-cmd.h"
#include "ur.h"

int
main (int argc, char ** argv)
{
  bool recursive = false;
  char *path = NULL;

  if (argc == 3) {
    if (strcmp (argv[1], "-r") != 0) {
      printf ("usage: ur-add [-r] path\n");
      return -1;
    }  
    else {
      recursive = true;
      path = argv[2];
    }
  }
  
  else if (argc == 2) {    
      path = argv[1];
  }
  
  else {
    printf ("usage: ur-add [-r] path\n");
    return -1;
  }

  init_ur ();

  return cmd_add (path, recursive);      
}
