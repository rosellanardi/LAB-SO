#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "syscall.h"
#include "semaphore.h"

#define N 256

struct semaphore {
  struct spinlock lock;
  int count;
  int bound;
};

struct semaphore semaphores[N];

void 
semaphores_init(void)
{
  for(unsigned int i = 0; i < N; i++) {
    (&semaphores[i])->count = -1;
    (&semaphores[i])->bound = -1;
    initlock(&(&semaphores[i])->lock, "semaphorelock");
  }  
}

int 
get_sem(int value)
{
  int index = 0;

  while((sem_open(index, value) != 1) && (index < N)) {
    index++;  
  }
  
  if (index == N) {
    return -1;
  }
  
  return index;
}

int 
sem_open(int sem, int value)
{

  if (sem < 0 || sem >= N) {
    return 0;
  }

  struct semaphore *open_sem = &semaphores[sem];
  
  acquire(&open_sem->lock);
  
  if (open_sem->count != -1) {
    release(&open_sem->lock);
    return 0;
  }
  
  open_sem->count = value;
  open_sem->bound = value;
    
  release(&open_sem->lock);
  
  return 1;
}

int
sem_close(int sem)
{
  
  if (sem < 0 || sem >= N) {
    return 0;
  }

  struct semaphore *close_sem = &semaphores[sem];
  
  acquire(&close_sem->lock);
  
  if (close_sem->count == -1) {
    release(&close_sem->lock);
    return 0;
  }
  
  close_sem->count = -1;
  
  release(&close_sem->lock);
  
  return 1;
}

int
sem_up(int sem)
{

  if (sem < 0 || sem >= N) {
    return 0;
  }

  struct semaphore *up_sem = &semaphores[sem];

  acquire(&up_sem->lock);
  
  if (up_sem->count == -1) {
    release(&up_sem->lock);
    return 0;
  }
  
  if (up_sem->count == up_sem->bound) {
    release(&up_sem->lock);
    return 0;
  }
  
  up_sem->count = up_sem->count + 1;

  wakeup(up_sem);

  release(&up_sem->lock);
  
  return 1;
}

int
sem_down(int sem)
{
  
  if (sem < 0 || sem >= N) {
    return 0;
  }

  struct semaphore *down_sem = &semaphores[sem];

  acquire(&down_sem->lock);

  if (down_sem->count == -1) {
    release(&down_sem->lock);
    return 0;
  }

  while(down_sem->count == 0) {
    sleep(down_sem , &down_sem->lock);
  }

  down_sem->count = down_sem->count - 1;

  release(&down_sem->lock);
  
  return 1;  
}