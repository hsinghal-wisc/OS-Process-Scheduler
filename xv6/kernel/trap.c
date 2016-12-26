#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;
uint ticks2;
/*
void
check(void)
{

if(ticks==100){
 for(i=0;i<c1;i++)
 {
 if(q1[i]->state != RUNNABLE)
continue;
else {
 if(q1[i]->used == 0) {

     //copy proc to lower priority queue
 c0++;
  proc->priority=proc->priority-1;
  pstat_var.priority[proc->pid] = proc->priority;
  q0[c0] = proc;
               //delete proc from q0
                q1[i]=NULL;
   for(j=i;j<=c1-1;j++)
      q1[j] = q1[j+1];
    q1[c1] = NULL;
             proc->clicks = 0;
              c1--;
 cprintf("bhasad id =%d name =%s priority =%d clicks =%dticks =%d\n",proc->pid,proc->name,proc->priority,proc->clicks,ticks);
  }
  else
  q1[i]->used = 0;
  }
  }
  for(i=0;i<c2;i++)
  {
 if(q2[i]->state != RUNNABLE)
  continue;
 else {
  if(q2[i]->used == 0) {
 
      //copy proc to lower priority queue
 c1++;
  proc->priority=proc->priority-1;
  pstat_var.priority[proc->pid] = proc->priority;
  q1[c1] = proc;
               //delete proc from q0
                 q2[i]=NULL;
     for(j=i;j<=c2-1;j++)
        q2[j] = q2[j+1];
    q2[c2] = NULL;
              proc->clicks = 0;
                c2--;
cprintf("bhasad id =%d name =%s priority =%d clicks =%dticks =%d\n",proc->pid,proc->name,proc->priority,proc->clicks,ticks);
  }
  else
  q2[i]->used = 0;
  }
  }
  for(i=0;i<c3;i++)
  {
 if(q3[i]->state != RUNNABLE)
  continue;
  else {
 if(q3[i]->used == 0) {
 
      //copy proc to lower priority queue
   c2++;
   proc->priority=proc->priority-1;
  pstat_var.priority[proc->pid] = proc->priority;
   q2[c2] = proc;
                //delete proc from q0
                 q3[i]=NULL;
   for(j=i;j<=c3-1;j++)
      q3[j] = q3[j+1];
     q3[c3] = NULL;
              proc->clicks = 0;
              c3--;
cprintf("bhasad id =%d name =%s priority =%d clicks =%dticks =%d\n",proc->pid,proc->name,proc->priority,proc->clicks,ticks);
 }
 else
 q3[i]->used = 0;
 }
 }
 ticks=0;
 }
}
*/
void 
check(void);

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
