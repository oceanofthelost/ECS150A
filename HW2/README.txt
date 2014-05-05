ID: 998152950 Name: Sean Alling 
ID: 999403734 Name: Bradon Maynard
kern_switch.c  120, 121, 859-939
proc.h 62, 514, 515
kern_fork.c 785-786

proc.h: has the added attributes from HW1

kern_fork.c: has a method on line 785 to 786 that sets the default value for the tickets for every thread. It is best to initialize every process with a value for tickets. If you do not initialize the tickets yet use the value you run the the chance of tickets getting the value in memory that the location had before we assigned the location to the ticket value. So we set all tickets to 4 when the process is created. 

kern_switch.c: This is the file with all of the scheduling code. The line that really matters if((lottery_mode == 1) && (pri>=40 && pri<56)). What this line does is checks to see if the current process that we have in the scheduled is in the time share value region (pri>=40 to pri =55) and that lottery mode is set. If the process is not in the timeshare region then regular scheduling will be used. If we do have a process that has a timeshare priority and lottery mode is set we enter the lottery scheduler. First all the tickets that are in timeshare are added up. Then a random number mod tickets is generated after which we then iterate over all the timeshare process and subtract there ticket amount from the random number. We subtract till we get a value that is less than 0 in which case we execute that process. Setting all process to a default value will not alter the running of lottery mode. In lottery mode we are only ever looking at the ticket values in process in the timeshare region and never in any other region.

Contrabutions: 

Brandon Maynard: Did the research to find the macros that we use to work with runtime queues. Then found out how we are able to access the ticket value element in each process from within runq_
choose(using kse_ptr->ke_thread->td_proc->tickets;). Also wrote out the psudocode for lottery mode.

Sean Alling: Programmed the lottery mode into kern_switch.c. Added the code to kern_fork.c to give each process a default value for tickets. Supplied all the files from HW1 and also tested to make sure that lottery scheduling did indeed work. Also implemented social aware scheduling before when it was still required before the requirements changed. 
