#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct pstat {
  int inuse[NPROC]; 
  int pid[NPROC];   
  enum procstate state[NPROC];
  int ticks[NPROC][4]; 
  int priority[NPROC]; 
};


#endif // _PSTAT_H_
