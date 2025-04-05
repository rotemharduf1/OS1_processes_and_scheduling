#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  struct proc *p = myproc();
  int MSG_SIZE = sizeof(p->exit_msg);

  int status;
  char msg[MSG_SIZE];

  argint(0, &status);

  if(argstr(1, msg, MSG_SIZE) < 0) { // if no message is provided
    printf("No message provided, using default empty.\n");
    msg[0] = '\0'; // set default message to empty string
  }

  strncpy(p->exit_msg, msg, MSG_SIZE - 1);
  p->exit_msg[MSG_SIZE - 1] = '\0';
  // print the exit message
  printf("Process %d exiting with message: %s\n", p->pid, p->exit_msg);

  exit(status);
  return 0;  // Not reached
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
  uint64 p;
  argaddr(0, &p);
  return wait(p);
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