#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SIZE 65536
#define MAX_CHILDREN 16

int array[SIZE];

// Convert int to string
void int_to_str(int x, char *s) {
  char buf[16];
  int i = 0;

  if (x == 0) {
    s[0] = '0';
    s[1] = '\0';
    return;
  }

  while (x > 0) {
    buf[i++] = (x % 10) + '0';
    x /= 10;
  }

  for (int j = 0; j < i; j++)
    s[j] = buf[i - j - 1];
  s[i] = '\0';
}

void run_child(int index, int n) {
  int chunk_size = SIZE / n;
  int start = index * chunk_size;
  int end = (index == n - 1) ? SIZE : start + chunk_size;

  for (int i = 0; i < SIZE; i++)
    array[i] = i;

  int sum = 0;
  for (int i = start; i < end; i++)
    sum += array[i];

  printf("Child #%d (pid %d): sum = %d\n", index + 1, getpid(), sum);
  sleep(5);
  write(1, "", 0);
  exit(sum, "");
}

int main(int argc, char *argv[]) {
  int n = 4;

  // Child exec mode
  if (argc == 3 && strcmp(argv[1], "child") == 0) {
    int index = atoi(argv[2]);
    run_child(index, n);
    exit(1, "unreachable");
  }

  // Parent main mode
  if (argc == 2)
    n = atoi(argv[1]);

  if (n < 1 || n > MAX_CHILDREN) {
    printf("Invalid number of processes. Must be between 1 and %d\n", MAX_CHILDREN);
    exit(1, "");
  }

  int pids[MAX_CHILDREN];
  int fork_result = forkn(n, pids);

  if (fork_result < 0) {
    printf("forkn failed\n");
    exit(1, "");
  }

  if (fork_result == 0) {
    // Parent process
    printf("Parent: created %d children with PIDs: ", n);
    for (int i = 0; i < n; i++)
      printf("%d ", pids[i]);
    printf("\n");

    sleep(10);  // allow children to exit cleanly

    int count = 0;
    int statuses[MAX_CHILDREN];

    if (waitall(&count, statuses) < 0) {
      printf("waitall failed\n");
      exit(1, "");
    }

    if (count != n) {
      printf("Error: waitall expected %d children, got %d\n", n, count);
      exit(1, "");
    }

    int total = 0;
    for (int i = 0; i < count; i++)
      total += statuses[i];

    printf("Final total sum: %d\n", total);
    printf("Expected total: 2147450880\n");

    exit(0, "done");
  } else {
    // Each child re-execs itself
    char index_str[8];
    int_to_str(fork_result - 1, index_str);
    char *args[] = {"bigarray", "child", index_str, 0};
    exec("bigarray", args);
    printf("exec failed\n");
    exit(1, "exec fail");
  }
}
