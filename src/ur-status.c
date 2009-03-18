#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ur-cmd.h"
#include "ur.h"

int
main (int argc, char ** argv)
{
  init_ur ();

  if (argc != 1) {
    printf ("usage: ur-status\n");
    return -1;
  }

  return cmd_status (".", true);    
  
  return 0;
}
