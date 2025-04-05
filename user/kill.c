#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int i;

  if(argc < 2){
    fprintf(2, "usage: kill pid...\n");
    exit_wrapper(1);
  }
  for(i=1; i<argc; i++)
    kill(atoi(argv[i]));
  exit_wrapper(0);
  return 0; // Ensure main returns an integer
}
