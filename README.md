# OS-Process-Scheduler
To implement a basic MLFQ scheduler.
To show how process behavior (i.e., how long a process uses the CPU before performing I/O or sleeping) interacts with the scheduler by creating an interesting timeline graph.

Implement MLFQ

Your MLFQ scheduler must follow these very precise rules:

Four priority levels, numbered from 0 (highest) down to 3 (lowest).
Whenever the xv6 10 ms timer tick occurs, the highest priority ready process is scheduled to run.
The highest priority ready process is scheduled to run whenever the previously running process exits, sleeps, or otherwise yields the CPU.
When a timer tick occurs, whichever process was currently using the CPU should be considered to have used up an entire timer tick's worth of CPU. (Note that a timer tick is different than the time-slice.)
The time-slice associated with priority 0 is 5 timer ticks; for priority 1 it is also 5 timer ticks; for priority 2 it is 10 timer ticks, and for priority 3 it is 20 timer ticks.
When a new process arrives, it should start at priority 0.
If no higher priority job arrives and the running process does not relinquish the CPU, then that process is scheduled for an entire time-slice before the scheduler switches to another process.
At priorities 0, 1, and 2, after a process consumes its time-slice it should be downgraded one priority. After a time-slice at priority 3, the CPU should be allocated to a new process (i.e., use Round Robin with a 200~ms time-slice cross processes that are all at priority 3).
If a process voluntarily relinquishes the CPU before its time-slice expires, its time-slice should not be reset; the next time that process is scheduled, it will continue to use the remainder of its existing time-slice.
After each 1 second interval, if a runnable process has not been scheduled at all in that interval, its priority should be bumped up by one level and given a new time-slice. Note that this 1 second interval is system-wide; at this same point in time, every runnable process is evaluated for starvation
