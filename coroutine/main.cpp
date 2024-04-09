#include <iostream>
#include <setjmp.h>

/*
ucontext 是 POSIX 标准中提供的一组函数，用于管理用户级线程的上下文（context）。
用户级线程是由用户空间的线程库而不是内核调度的线程。
ucontext 可以允许程序员在用户级别实现线程调度和上下文切换，而无需依赖于操作系统提供的线程调度器。
    getcontext：获取当前线程的上下文，并将其保存到指定的 ucontext_t 结构体中。
    setcontext：根据给定的 ucontext_t 结构体，设置当前线程的上下文，实现上下文切换。
    makecontext：创建一个新的上下文，指定其执行函数、堆栈和参数。
    swapcontext：保存当前线程的上下文，并将执行流切换到另一个上下文。
*/
#include <ucontext.h>

ucontext_t ctx[2];
ucontext_t main_ctx;

int count = 0;
void fun_1()
{
    while(count++ < 3)
    {
        std::cout<<"fun_1 beg:"<< count <<"\n";
        swapcontext(ctx, ctx + 1);
        std::cout<<"fun_1 end:"<< count <<"\n";
    }
    std::cout<<"fun_1 end:\n";
}
void fun_2()
{
    while(count++ < 3)
    {
        std::cout<<"fun_2 beg:"<< count <<"\n";
        swapcontext(ctx + 1, ctx);
        std::cout<<"fun_2 end:"<< count <<"\n";
    }
    std::cout<<"fun_2 end:\n";
}

#define  stack_size 4096
int main(int argc, char * argv[]) 
{
    // main1(0, 0);
    std::cout<<argc<<":"<<argv[0]<<"\n";

    char stack1[stack_size] = {};
    char stack2[stack_size] = {};
    getcontext(ctx);
    ctx[0].uc_stack.ss_sp = stack1;
    ctx[0].uc_stack.ss_size = sizeof(stack1);
    ctx[0].uc_link = &main_ctx;
    makecontext(ctx, fun_1, 0);

    getcontext(ctx + 1);
    ctx[1].uc_stack.ss_sp = stack2;
    ctx[1].uc_stack.ss_size = sizeof(stack2);
    ctx[0].uc_link = &main_ctx;
    makecontext(ctx + 1, fun_2, 0);

    swapcontext(&main_ctx, ctx);

    return 0;
}

jmp_buf env;

void func(int arg)
{
    std::cout<<arg<<"\n";
    longjmp(env, ++arg);
}


int main1(int argc, char * argv[]) 
{
    int ret = setjmp(env); //不利于对线程之间跳转 跨栈
    if(ret == 0)
    {
        func(ret);
    }else  if(ret == 1)
    {
        func(ret);
    }else  if(ret == 2)
    {
        func(ret);
    }
    return 0;
}

