/*********************
Name:   Sean Alling
ID:     998152950 
********************/
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/mutex.h>

/*
int setProcessTickets(int pid, int tickets);
int getProcessTickets(int pid); 
int setSocialInfo(int pid, u_int64_t social_info);
u_int64_t getSocialInfo(int pid);
int setLotteryMode(int mode);
int getLotteryMode(void); 
*/



/*
    we are declaring all of the args structs for each 
    of the 6 processes
 */
struct setProcessTickets_args
{
    int pid; 
    int tickets;
};

struct getProcessTickets_args
{
    int pid; 
};

struct setSocialInfo_args
{
    int pid;
    u_int64_t social_info;
};

struct getSocialInfo_args
{
    int pid;
};

struct setLotteryMode_args
{
    int mode;
};


/*
    we declare the function for the system calls
 */
static int setProcessTickets(struct thread *td,struct setProcessTickets_args *arg)
{
    //get value that we will set the process tickets
    struct proc *Process = pfind(arg->pid);
    
    //if Process is NULL then there is an error. 
    //so we have to return a -1
    if(Process == NULL)
    {
        td -> td_retval[0] = -1;
    }
    else
    {
        //tickets are equat to or greater than 1
        if(arg->tickets >= 1)
        {
            Process -> tickets = arg -> tickets;
            PROC_UNLOCK(Process);
            td -> td_retval[0] = Process -> tickets;
        }
        else //tickets were equalt to or less than 0
        {
            Process -> tickets = 1;
            PROC_UNLOCK(Process);
            td -> td_retval[0] = 1;
        }
    }
    return 0;
}

static int getProcessTickets(struct thread *td,struct getProcessTickets_args *arg)
{
    //get value that we will set the process tickets
    struct proc *Process = pfind(arg->pid);
    
    //if Process is NULL then there is an error. 
    //so we have to return a -1
    if(Process == NULL)
    {
        td -> td_retval[0] = -1;
    }
    else
    {
        int NumberTickets = Process -> tickets;
        PROC_UNLOCK(Process);
        td -> td_retval[0] = NumberTickets;
    }
    return 0;
}

static int setSocialInfo(struct thread *td,struct setSocialInfo_args *arg)
{
    //get value that we will set the process tickets
    struct proc *Process = pfind(arg->pid);
    
    //if Process is NULL then there is an error. 
    //so we have to return a -1
    if(Process == NULL)
    {
        td -> td_retval[0] = -1;
    }
    else
    {
        Process -> social_info = arg -> social_info;
        PROC_UNLOCK(Process);
        td -> td_retval[0] = Process -> social_info;
    }
    return 0;
}

static int getSocialInfo(struct thread *td,struct getSocialInfo_args *arg)
{
    //get value that we will set the process tickets
    struct proc *Process = pfind(arg->pid);
    
    //if Process is NULL then there is an error. 
    //so we have to return a -1
    if(Process == NULL)
    {
        td -> td_retval[0] = -1;
    }
    else
    {
        u_int64_t  SocialInformation = Process -> social_info;
        PROC_UNLOCK(Process);
        td -> td_retval[0] = SocialInformation;
    }
    return 0;
}


static int setLotteryMode(struct thread *td, struct setLotteryMode_args *arg)
{
    lottery_mode = arg -> mode;
    td -> td_retval[0] = lottery_mode;
    return 0;
}

static int getLotteryMode(struct thread *td, void *arg)
{
    td -> td_retval[0] = lottery_mode;
    return 0;
}


/*
    we define the sysent datastructor for each system call
 */
static struct sysent setProcessTickets_sysent = {2,setProcessTickets};
static struct sysent getProcessTickets_sysent = {1,getProcessTickets};
static struct sysent setSocialInfo_sysent     = {2,setSocialInfo};
static struct sysent getSocialInfo_sysent     = {1,getSocialInfo};
static struct sysent setLotteryMode_sysent    = {1,setLotteryMode};
static struct sysent getLotteryMode_sysent    = {0,getLotteryMode};


/* 
    we get the offset value for each of the system calls
*/
static int setProcessTickets_offset = NO_SYSCALL;
static int getProcessTickets_offset = NO_SYSCALL;
static int setSocialInfo_offset     = NO_SYSCALL;
static int getSocialInfo_offset     = NO_SYSCALL;
static int setLotteryMode_offset    = NO_SYSCALL;
static int getLotteryMode_offset    = NO_SYSCALL;

/*
    we define the laod function for each of the system call
    in order to use the KLD interface. 
*/

static int setProcessTickets_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", setProcessTickets_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", setProcessTickets_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}

static int getProcessTickets_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", getProcessTickets_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", getProcessTickets_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}

static int setSocialInfo_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", setSocialInfo_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", setSocialInfo_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}

static int getSocialInfo_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", getSocialInfo_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", getSocialInfo_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}

static int setLotteryMode_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", setLotteryMode_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", setLotteryMode_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}

static int getLotteryMode_load(struct module *m, int what, void *arg)
{
    int error = 0;

    switch (what) 
    {
        case MOD_LOAD:
            printf("System call loaded at slot: %d\n", getLotteryMode_offset);
            break;
        case MOD_UNLOAD:
            printf("System call unloaded from slot: %d\n", getLotteryMode_offset);
            break;
        default:
            error = EINVAL;
            break;
      }
      return error;
}


SYSCALL_MODULE(setProcessTickets,&setProcessTickets_offset,&setProcessTickets_sysent,setProcessTickets_load,NULL);
SYSCALL_MODULE(getProcessTickets,&getProcessTickets_offset,&getProcessTickets_sysent,getProcessTickets_load,NULL);
SYSCALL_MODULE(setSocialInfo,&setSocialInfo_offset,&setSocialInfo_sysent,setSocialInfo_load,NULL);
SYSCALL_MODULE(getSocialInfo,&getSocialInfo_offset,&getSocialInfo_sysent,getSocialInfo_load,NULL);
SYSCALL_MODULE(setLotteryMode,&setLotteryMode_offset,&setLotteryMode_sysent,setLotteryMode_load,NULL);
SYSCALL_MODULE(getLotteryMode,&getLotteryMode_offset,&getLotteryMode_sysent,getLotteryMode_load,NULL);
