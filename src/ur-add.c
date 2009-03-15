#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ur-cmd.h"
#include "ur.h"

int
main (int argc, char ** argv)
{
  init_ur ();

  if (argc != 2) {
    printf ("usage: ur-add path\n");
    return -1;
  }

  return cmd_add (argv[1], true);    
  
  return 0;
}
