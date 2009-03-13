#include <stdlib.h>
#include <stdio.h>

#include "cmd-init.h"

int
main (int argc, char ** argv)
{
  if (argc != 1) {
    printf ("usage:\nur-init\n");
    return -1;
  }

  return cmd_init (".");
}
