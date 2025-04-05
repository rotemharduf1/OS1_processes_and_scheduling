#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SIZE 65536      // 2^16 elements
#define MAX_CHILDREN 16 // upper limit allowed by forkn

int array[SIZE];

int
main(int argc, char *argv[])
{
    int n = 4;

    if (argc == 2)
        n = atoi(argv[1]);

    if (n < 1 || n > MAX_CHILDREN) {
        printf("Invalid number of processes. Must be between 1 and %d\n", MAX_CHILDREN);
        exit(1, "");
    }

    // Fill array with consecutive numbers
    for (int i = 0; i < SIZE; i++)
        array[i] = i;

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
        // Child process: fork_result = 1 to n
        int child_index = fork_result - 1;
        int chunk_size = SIZE / n;
        int start = child_index * chunk_size;
        int end = start + chunk_size;
        if (child_index == n - 1)
            end = SIZE;  // last child may pick up the remainder

        int sum = 0;
        for (int i = start; i < end; i++)
            sum += array[i];

        printf("Child #%d (pid %d): sum = %d\n", child_index + 1, getpid(), sum);
        exit(sum, "");
    }
}
