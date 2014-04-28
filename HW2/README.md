HW2 for ECS150: Lottery and Social Awaare Scheduling

Files: 
	hw1.c: Contains 6 system calls for freeBSD

	proc.h: Added parameters to the proc structure and an extern variable lottey_mode

	kern_fork.c: Added code to set the default tickets to 4 in fork_exit

	kern_switch.c: Defined a global variable called lottery mode that determins the scheduling mode that we will be using. Also added two new types of scheduling, Lottery Schduling and Social Aware Scheduling. 

	Makefile: Used to compile hw1.c

	Steps to compile on freeBSD:

	1. use scp to transfer all files to a single folder on your freeBSD station.

	2. Logon to freeBSD and cd to the directory you just transfered the files above to

	3. issue the following commands to bakup files
	cp /usr/src/sys/sys/proc.h /usr/src/sys/sys/proc.h.bkp
	cp /usr/src/sys/kern/kern_switch.c /usr/src/sys/kern/kern_switch.c.bkp
	cp /usr/src/sys/kern/kern_fork.c /usr/src/sys/kern/kern_fork.c.bkp
	mv ./proc.h /usr/src/sys/sys/proc.h
	mv ./kern_switch.c /usr/src/sys/kern/kern_switch.c
	mv ./kern_fork.c /usr/src/sys/kern/kern_fork.c

	4. Compile the kernel, Assumes ycd to the folder ou have already backup the kernel
	cd /usr/src/sys/i386/compile/MYKERNEL
	make depend
	make 
	make install
	reboot

	5. Compile and load System Calls
	cd to the folder you transfered files to earlier then issue the following commands
	make
	kldload ./hw1.ko

	6. Use lottcheck
	if you have transfered lottcheck you can now use it to test Lottery Scheduling. 