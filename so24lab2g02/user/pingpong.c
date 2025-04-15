#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
pingpong(int amount)
{
  int var = 0;
  int index = 0;
  int sems[2] = {-1, -1};
     
  while((var != 2) && (index < 256)) {
    if (sem_open(index,1) == 1) {
      sems[var] = index;
      var++;
    }
    index++;
  }
    
  int sem1 = sems[0];
  int sem2 = sems[1];
  
  if ((sem1 == -1) || (sem2 == -1)) {
    printf("ERROR: sem_open falló\n");
    exit(1);
  }
     
  sem_down(sem2);
  int rc = fork();
  
  if (rc < 0) {
    printf("ERROR: en proceso hijo \n");
  }
    
  unsigned int i = 0;
  while(i < amount) {
    if (rc==0) {
      sem_down(sem1);
      printf("Ping\n");
      sem_up(sem2);
    } else {
      sem_down(sem2);
      printf("          Pong\n");
      sem_up(sem1);
    }
    i = i + 1;
  }
  wait(0);
  sem_close(sem1);
  sem_close(sem2);

  exit(0);
}

int
main(int argc, char *argv[])
{
  if(argc != 2) {
    printf("ERROR: El número de argumentos es incorrecto\n");
    exit(1);
  }

  int amount = atoi(argv[1]);

  if (amount < 1) {
    printf("ERROR: El número de rounds tiene que ser mayor a 1\n");
    exit(1);
  }
  
  pingpong(amount);
  exit(0);
}
