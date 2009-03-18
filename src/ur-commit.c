#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "ur-cmd.h"
#include "ur.h"

int
main (int argc, char ** argv)
{
  bool all = false, recursive = false;
  char *msg = NULL;
  int ch;

  init_ur ();

  while ((ch = getopt (argc, argv, "arm:")) != -1) {
    switch (ch) {
    case 'a':
      all = true;
      break;
    case 'r':
      recursive = true;
      break;
    case 'm':
      msg = optarg;
      break;
    case '?':
      printf ("usage: ur-commit [-ar] [-m <msg>]\n");
      return -1;
      break;
    }
  }

  return cmd_commit (".", recursive, all, msg);    
  
  return 0;
}
