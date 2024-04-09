#include <iostream>
#include <ucontext.h>

ucontext_t ctx[2];
ucontext_t main_ctx;


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
    //makecontext(ctx, fun_1, 0);

    getcontext(ctx + 1);
    ctx[1].uc_stack.ss_sp = stack2;
    ctx[1].uc_stack.ss_size = sizeof(stack2);
    ctx[0].uc_link = &main_ctx;
    //makecontext(ctx + 1, fun_2, 0);

    swapcontext(&main_ctx, ctx);

    return 0;
}

