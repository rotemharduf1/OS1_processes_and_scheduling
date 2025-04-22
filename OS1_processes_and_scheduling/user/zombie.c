// Create a zombie process that
// must be reparented at exit_wrapper.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  if(fork() > 0)
    sleep(5);  // Let child exit_wrapper before parent.
  exit(0, "");
}
