ID: 998152950 Name: Sean Alling 
ID: 999403734 Name: Brandon Maynard
kern_switch.c  120, 121, 859-939
proc.h 62, 514, 515
kern_fork.c 785-786

proc.h: has the added attributes from HW1

kern_fork.c: has a method on line 785 to 786 that sets the default value for the tickets for every thread. It is best to initialize every process with a value for tickets. If you do not initialize the tickets and use the value, you run the chance of tickets getting the value in memory that the location had before we assigned the location to the ticket value. Because of this, we set all tickets to value 4 when the process is created. 

kern_switch.c: This is the file with all of the scheduling code. The line that really matters is: if((lottery_mode == 1) && (pri>=40 && pri<56)). This line checks to see if the current process that we have scheduled is in the timeshare value region (pri>=40 to pri =55) and that lottery mode is set. If the process is not in the timeshare region, then regular scheduling will be used. If we do have a process that has a timeshare priority and lottery mode is set, we enter the lottery scheduler. First, all the tickets that are in timeshare are added up. A random number mod tickets is then generated. We then iterate over all the timeshare process and subtract the ticket amount from the random number. We subtract until we get a value that is less than 0 in which case we execute that process. Setting all processes to a default value will not alter the running of lottery mode. In lottery mode we are only looking at the ticket values in processes in the timeshare region and never in any other region.

Contributions: 

Brandon Maynard: Did the research to find the macros that we used to work with runtime queues. Found out how we are able to access the ticket value element in each process from within runq_choose(using kse_ptr->ke_thread->td_proc->tickets;). Wrote out the pseudocode for lottery mode.

Sean Alling: Used the pseudocode to program the lottery mode into kern_switch.c. Added the code to kern_fork.c to give each process a default value for tickets. Supplied the files from HW1 and also tested to make sure that lottery scheduling did indeed work. 

At different times, we got together and worked on the homework, but we are splitting up the contribution to the project to satisfy the requirement of the README.  We tested the program together on both of our computers to verify its function, and we wrote this README file together.