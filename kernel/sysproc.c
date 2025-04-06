#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct proc proc[NPROC];
extern struct spinlock wait_lock;
extern void public_freeproc(struct proc *p);

uint64
sys_exit(void)
{
  int n;
  char msg[32];
  char def_msg[32]="No msg provided";

  argint(0, &n);
  argstr(1, msg, 32);

  if(msg == 0)
    exit(n,def_msg);
  else
    exit(n,msg);
  return 0;
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 status_adder;
  uint64 msg_adder;

  argaddr(0, &status_adder);
  argaddr(1, &msg_adder);

  return wait(status_adder, msg_adder);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//task2:
//outputs the size of the running process’ memory in bytes and a userspace program to test it
uint64
sys_memsize(void)
{
  uint64 memsize;
  struct proc *p = myproc();
  memsize = p->sz;
  return memsize;
}

//task4:
//forkn: fork a process n times
uint64
sys_forkn(void) {
  int n;
  uint64 pids;

  argint(0, &n);
  argaddr(1, &pids);

  if(n < 1 || n > 16)  return -1;

  struct proc *p = myproc();
  int created = 0;
  int pid_array[n];

  for (int i = 0; i < n; i++) {
    int pid = fork();
    if (pid < 0) {
      // Cleanup if fork fails
      for (int j = 0; j < created; j++)
        kill(pid_array[j]);
      return -1;
    }
    if (pid == 0) {
      // Child process
      return i + 1;
    }
    pid_array[i] = pid;
    created++;
  }

  if (copyout(p->pagetable, pids, (char*)pid_array, n * sizeof(int)) < 0)
    return -1;

  return 0; // Success
}

//task4:
//waitall: wait for all children to exit
uint64
sys_waitall(void) {
  uint64 n_addr, statuses_addr;
  int count = 0;
  int statuses[NPROC];

  argaddr(0, &n_addr);
  argaddr(1, &statuses_addr);

  struct proc *p = myproc();

  for (;;) {
    struct proc *pp;
    int found = 0;

    acquire(&wait_lock);
    for (pp = proc; pp < &proc[NPROC]; pp++) {
      if (pp->parent == p && pp->state == ZOMBIE) {
        found = 1;
        statuses[count++] = pp->xstate;
        acquire(&pp->lock); // Acquire the child process lock before freeing
        release(&wait_lock); // Release wait_lock before calling freeproc
        public_freeproc(pp);
        release(&pp->lock);
        break;
      }
    }
    if (!found) {
      release(&wait_lock);
      break;
    }
  }

  printf("sys_waitall: Processed %d children\n", count); // Debugging print

  if (copyout(p->pagetable, n_addr, (char*)&count, sizeof(int)) < 0)
    return -1;

  if (copyout(p->pagetable, statuses_addr, (char*)statuses, count * sizeof(int)) < 0)
    return -1;

  return 0;
}


