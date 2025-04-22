#include "kernel/types.h"
#include "user/user.h"
#include "kernel/memlayout.h"

int main(int argc, char *argv[]){
  int initSize = memsize();
  printf("The Initial memory size is: %d bytes\n", initSize);

  char *memory = malloc(200000);
  if(memory == 0){
    printf("Memory allocation failed\n");
    exit(1, "");
  }

  int afterMalloc = memsize();
  printf("The memory size after malloc is: %d bytes\n", afterMalloc);

  free(memory);

  int afterFree = memsize();
  printf("The memory size after free is: %d bytes\n", afterFree);

  exit(0, "");
}