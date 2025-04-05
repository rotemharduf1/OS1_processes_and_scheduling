// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define N  1000

void
print(const char *s)
{
  write(1, s, strlen(s));
}

void
forktest(void)
{
  int n, pid;

  print("fork test\n");

  for(n=0; n<N; n++){
    pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit_wrapper(0);
  }

  if(n == N){
    print("fork claimed to work N times!\n");
    exit_wrapper(1);
  }

  for(; n > 0; n--){
    if(wait_wrapper(0) < 0){
      print("wait_wrapper stopped early\n");
      exit_wrapper(1);
    }
  }

  if(wait_wrapper(0) != -1){
    print("wait_wrapper got too many\n");
    exit_wrapper(1);
  }

  print("fork test OK\n");
}

int
main(void)
{
  forktest();
  exit_wrapper(0);
  return 0; // Ensure main returns an integer

}
