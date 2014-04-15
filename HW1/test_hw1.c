#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

int test_setProcessTickets(int p, int t, int syscall_num)
{
    return syscall(syscall_num, p, t);
}

int test_getProcessTickets(int p, int syscall_num)
{
    return syscall(syscall_num, p);
}

int test_setSocialInfo(int p, u_int64_t t, int syscall_num)
{
    return syscall(syscall_num, p, t);
}

u_int64_t test_getSocialInfo(int p, int syscall_num)
{
    return syscall(syscall_num, p);
}

int test_setLotteryMode(int t, int syscall_num)
{
    return syscall(syscall_num, t);
}

int test_getLotteryMode(int syscall_num)
{
    return syscall(syscall_num);
}

int main(void)
{
    int syscall_num[6];
    struct module_stat stat;
    stat.version = sizeof(stat);
 
    modstat(modfind("setProcessTickets"), &stat);
    syscall_num[0] = stat.data.intval;
    int i;
    for(i=20; i<40; i+=2)
        test_setProcessTickets(i, i+1, syscall_num[0]);

    modstat(modfind("getProcessTickets"), &stat);
    syscall_num[1] = stat.data.intval;
    int j=0;
    for(i=20; i<=40; i+=2)
    {
        j = test_getProcessTickets(i, syscall_num[1]);
        if( j != -1)
            printf("pid=%d exists, tickets=%d\n", i, j);
        else
            printf("pid=%d does not exists, tickets=%d\n", i, j);
    }
    
    modstat(modfind("setSocialInfo"), &stat);
    syscall_num[2] = stat.data.intval;
    int p;
    for(p=40; p<60; p+=2)
        test_setSocialInfo(p, 1ULL<<(p/2), syscall_num[2]);

    modstat(modfind("getSocialInfo"), &stat);
    syscall_num[3] = stat.data.intval;
    u_int64_t q=0;
    for(p=40; p<=60; p+=2)
    {
        q = test_getSocialInfo(p, syscall_num[3]);
        if( q != -1)
            printf("pid=%d exists, social_info=%Lu\n", p, q);
        else
            printf("pid=%d does not exists, social_info=%d\n", p, (int)q);
    }

    modstat(modfind("setLotteryMode"), &stat);
    syscall_num[4] = stat.data.intval;
    modstat(modfind("getLotteryMode"), &stat);
    syscall_num[5] = stat.data.intval;

    int mode;
    test_setLotteryMode(2, syscall_num[4]);
    mode = test_getLotteryMode(syscall_num[5]);
    printf("lotterymode is set to %d\n", mode);
    test_setLotteryMode(99, syscall_num[4]);
    mode = test_getLotteryMode(syscall_num[5]);
    printf("lotterymode is set to %d\n", mode);
    test_setLotteryMode(9, syscall_num[4]);
    mode = test_getLotteryMode(syscall_num[5]);
    printf("lotterymode is set to %d\n", mode);
    test_setLotteryMode(1, syscall_num[4]);
    mode = test_getLotteryMode(syscall_num[5]);
    printf("lotterymode is set to %d\n", mode);

    return 0;
}

