#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SIZE 65536
#define MAX_CHILDREN 16

int array[SIZE];
int printlock = 0;

// write formatted string directly using one write call
void print_line(const char *label, int num1, const char *label2, int num2, const char *label3, int num3) {
    char buf[128];
    char *p = buf;

    // convert integers to strings manually
    // very simple int->str function
    char tmp[20];
    int i, n;

    // part 1
    while (*label) *p++ = *label++;

    // num1
    n = num1;
    i = 0;
    if (n == 0) tmp[i++] = '0';
    else {
        while (n > 0) {
            tmp[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    while (i > 0) *p++ = tmp[--i];

    // part 2
    while (*label2) *p++ = *label2++;

    // num2
    n = num2;
    i = 0;
    if (n == 0) tmp[i++] = '0';
    else {
        while (n > 0) {
            tmp[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    while (i > 0) *p++ = tmp[--i];

    // part 3
    while (*label3) *p++ = *label3++;

    // num3
    n = num3;
    i = 0;
    if (n == 0) tmp[i++] = '0';
    else {
        while (n > 0) {
            tmp[i++] = '0' + (n % 10);
            n /= 10;
        }
    }
    while (i > 0) *p++ = tmp[--i];

    *p++ = '\n';

    // write all at once
    while (__sync_lock_test_and_set(&printlock, 1));
    write(1, buf, p - buf);
    __sync_lock_release(&printlock);
}

int main(int argc, char *argv[]) {
    int n = 4;

    if (argc == 2)
        n = atoi(argv[1]);

    if (n < 1 || n > MAX_CHILDREN) {
        printf("Invalid number of processes. Must be between 1 and %d\n", MAX_CHILDREN);
        exit(1, "");
    }

    for (int i = 0; i < SIZE; i++)
        array[i] = i;

    int pids[MAX_CHILDREN];
    int fork_result = forkn(n, pids);
    if (fork_result < 0) {
        printf("forkn failed\n");
        exit(1, "");
    }

    if (fork_result == 0) {
        // parent
        sleep(1);

        while (__sync_lock_test_and_set(&printlock, 1));
        printf("Parent: created %d children with PIDs:", n);
        for (int i = 0; i < n; i++)
            printf(" %d", pids[i]);
        printf("\n");
        __sync_lock_release(&printlock);

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

        while (__sync_lock_test_and_set(&printlock, 1));
        printf("Final total sum: %d\n", total);
        printf("Expected total: 2147450880\n");
        __sync_lock_release(&printlock);

        exit(0, "done");
    } else {
        // child
        int idx = fork_result - 1;
        int chunk = SIZE / n;
        int start = idx * chunk;
        int end = (idx == n - 1) ? SIZE : start + chunk;

        int sum = 0;
        for (int i = start; i < end; i++)
            sum += array[i];

        // 🧼 Clean, formatted, locked output
        print_line("Child #", idx + 1, " (pid ", getpid(), ") sum = ", sum);

        exit(sum, "");
    }
}
