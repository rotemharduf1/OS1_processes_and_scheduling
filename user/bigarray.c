#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SIZE 65536      // 2^16
#define MAX_CHILDREN 16 // maximun childran allowed
#define NPROC 64       // maximum number of processes

int main(void) {
    int pids[MAX_CHILDREN];
    int statuses[NPROC]; // Changed from MAX_CHILDREN to NPROC
    int chunk_size = SIZE / MAX_CHILDREN;
    int child_id = forkn(MAX_CHILDREN, pids);

    if (child_id == 0) {  // Parent Process
        int n, total_sum = 0;

        // Wait for all children and accumulate their sums
        if (waitall(&n, statuses) < 0) {
            printf("Error: waitall failed.\n");
            exit(1, "waitall failed");
        }

        printf("Number of children terminated: %d\n", n); // Debugging print

        for (int i = 0; i < n; i++) {
            printf("Child %d exited with status: %d\n", i, statuses[i]); // Debugging print
            total_sum += statuses[i];
        }

        printf("Total Sum: %d\n", total_sum);
        exit(0, "Calculation complete");
    }

    // Child process logic
    int index = child_id - 1;  // Fixing index calculation
    int start = index * chunk_size;
    int end = start + chunk_size;

    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += i;
    }

    printf("Child %d calculated sum: %d\n", child_id, sum); // Debugging print

    // Exit using the sum as the status
    exit(sum, "");
}