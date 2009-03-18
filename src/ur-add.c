#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ur-cmd.h"
#include "ur.h"

int
main (int argc, char ** argv)
{
  if (argc != 2) {
    printf ("usage: ur-add path\n");
    return -1;
  }

  init_ur ();

  return cmd_add (argv[1], true);      
}
