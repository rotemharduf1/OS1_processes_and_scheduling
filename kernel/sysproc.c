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
sys_forkn(void)
{
  int n;
  uint64 pids_addr;

  argint(0, &n);
  argaddr(1, &pids_addr);

  if (n < 1 || n > 16)
    return -1;

  struct proc *parent = myproc();
  struct proc *children[16];
  int pids[16];
  int created = 0;

  for (int i = 0; i < n; i++) {
    struct proc *np = custom_fork();
    if (!np) {
      for (int j = 0; j < created; j++) {
        acquire(&children[j]->lock);
        children[j]->killed = 1;
        release(&children[j]->lock);
      }
      return -1;
    }

    np->trapframe->a0 = i + 1; // set return value for child
    children[created] = np;
    pids[created] = np->pid;
    created++;
  }

  // Write pids back to user space
  if (copyout(parent->pagetable, pids_addr, (char *)pids, n * sizeof(int)) < 0)
    return -1;

  // Make all children runnable
  for (int i = 0; i < created; i++) {
    acquire(&children[i]->lock);
    children[i]->state = RUNNABLE;
    release(&children[i]->lock);
  }

  return 0;
}

//waitall: wait for all children to exit
uint64
sys_waitall(void)
{
  uint64 n_addr, statuses_addr;

  argaddr(0, &n_addr);
  argaddr(1, &statuses_addr);

  struct proc *p = myproc();
  struct proc *pp;
  struct proc *tofree[NPROC];
  int statuses[NPROC];
  int count;

  retry:
    count = 0;
  int found = 0;

  acquire(&wait_lock);

  for (pp = proc; pp < &proc[NPROC]; pp++) {
    if (pp->parent == p && pp != p) {
      found = 1;
      acquire(&pp->lock);
      if (pp->state == ZOMBIE) {
        statuses[count] = pp->xstate;
        tofree[count] = pp;
        count++;
        release(&pp->lock);
      } else {
        release(&pp->lock);
      }
    }
  }

  if (!found) {
    release(&wait_lock);
    int zero = 0;
    copyout(p->pagetable, n_addr, (char *)&zero, sizeof(int));
    return 0;
  }

  if (count < found) {
    release(&wait_lock);
    sleep((void*)1, &wait_lock);
    goto retry;
  }

  release(&wait_lock);

  for (int i = 0; i < count; i++) {
    acquire(&tofree[i]->lock);
    public_freeproc(tofree[i]);
    release(&tofree[i]->lock);
  }

  if (copyout(p->pagetable, n_addr, (char *)&count, sizeof(int)) < 0)
    return -1;

  if (copyout(p->pagetable, statuses_addr, (char *)statuses, sizeof(int) * count) < 0)
    return -1;

  return 0;
}
